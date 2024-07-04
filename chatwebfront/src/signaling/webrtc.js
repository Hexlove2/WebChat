import './style.css';

// 导入Firebase的核心模块和Firestore模块，用于数据库操作
import firebase from 'firebase/app';
import 'firebase/firestore';

// Firebase配置对象，包含与Firebase项目相关的配置信息
const firebaseConfig = {
  // your config
};

// 初始化Firebase应用，如果尚未初始化
if (!firebase.apps.length) {
  firebase.initializeApp(firebaseConfig);
}
// 获取Firestore实例，用于与Firebase Firestore数据库进行交互
const firestore = firebase.firestore();

// 配置ICE服务器，主要用于NAT穿透
const servers = {
  iceServers: [
    {
      urls: ['stun:stun1.l.google.com:19302', 'stun:stun2.l.google.com:19302'], // STUN服务器地址
    },
  ],
  iceCandidatePoolSize: 10, // ICE候选池的大小
};

// 全局状态变量
const pc = new RTCPeerConnection(servers); // 创建一个新的RTCPeerConnection对象
let localStream = null; // 本地媒体流
let remoteStream = null; // 远程媒体流

// 获取HTML页面中的元素
const webcamButton = document.getElementById('webcamButton');
const webcamVideo = document.getElementById('webcamVideo');
const callButton = document.getElementById('callButton');
const callInput = document.getElementById('callInput');
const answerButton = document.getElementById('answerButton');
const remoteVideo = document.getElementById('remoteVideo');
const hangupButton = document.getElementById('hangupButton');

// 1. 设置媒体源
webcamButton.onclick = async () => {
  // 获取本地媒体流（音频和视频）
  localStream = await navigator.mediaDevices.getUserMedia({ video: true, audio: true });
  remoteStream = new MediaStream(); // 初始化远程媒体流

  // 将本地流的轨道添加到RTCPeerConnection对象中
  localStream.getTracks().forEach((track) => {
    pc.addTrack(track, localStream);
  });

  // 当接收到远程流时，将其添加到远程媒体流中
  pc.ontrack = (event) => {
    event.streams[0].getTracks().forEach((track) => {
      remoteStream.addTrack(track);
    });
  };

  // 将本地流和远程流分别绑定到对应的视频元素上
  webcamVideo.srcObject = localStream;
  remoteVideo.srcObject = remoteStream;

  // 启用呼叫和应答按钮，禁用webcam按钮
  callButton.disabled = false;
  answerButton.disabled = false;
  webcamButton.disabled = true;
};

// 2. 创建一个Offer
callButton.onclick = async () => {
  // 获取Firestore集合的引用，用于信令交换
  const callDoc = firestore.collection('calls').doc();
  const offerCandidates = callDoc.collection('offerCandidates');
  const answerCandidates = callDoc.collection('answerCandidates');

  // 将呼叫文档的ID设置为输入框的值
  callInput.value = callDoc.id;

  // 当找到ICE候选时，将其保存到数据库中
  pc.onicecandidate = (event) => {
    event.candidate && offerCandidates.add(event.candidate.toJSON());
  };

  // 创建Offer描述
  const offerDescription = await pc.createOffer();
  await pc.setLocalDescription(offerDescription);

  const offer = {
    sdp: offerDescription.sdp,
    type: offerDescription.type,
  };

  // 将Offer保存到Firestore数据库中
  await callDoc.set({ offer });

  // 监听远程应答
  callDoc.onSnapshot((snapshot) => {
    const data = snapshot.data();
    if (!pc.currentRemoteDescription && data?.answer) {
      const answerDescription = new RTCSessionDescription(data.answer);
      pc.setRemoteDescription(answerDescription);
    }
  });

  // 当收到应答时，将候选者添加到对等连接中
  answerCandidates.onSnapshot((snapshot) => {
    snapshot.docChanges().forEach((change) => {
      if (change.type === 'added') {
        const candidate = new RTCIceCandidate(change.doc.data());
        pc.addIceCandidate(candidate);
      }
    });
  });

  // 启用挂断按钮
  hangupButton.disabled = false;
};

// 3. 使用唯一ID应答呼叫
answerButton.onclick = async () => {
  const callId = callInput.value; // 获取呼叫ID
  const callDoc = firestore.collection('calls').doc(callId); // 获取Firestore中的呼叫文档
  const answerCandidates = callDoc.collection('answerCandidates');
  const offerCandidates = callDoc.collection('offerCandidates');

  // 当找到ICE候选时，将其保存到数据库中
  pc.onicecandidate = (event) => {
    event.candidate && answerCandidates.add(event.candidate.toJSON());
  };

  const callData = (await callDoc.get()).data(); // 获取呼叫数据

  const offerDescription = callData.offer; // 获取Offer描述
  await pc.setRemoteDescription(new RTCSessionDescription(offerDescription)); // 设置远程描述

  const answerDescription = await pc.createAnswer(); // 创建应答描述
  await pc.setLocalDescription(answerDescription); // 设置本地描述

  const answer = {
    type: answerDescription.type,
    sdp: answerDescription.sdp,
  };

  // 更新Firestore数据库中的应答
  await callDoc.update({ answer });

  // 当收到Offer时，将候选者添加到对等连接中
  offerCandidates.onSnapshot((snapshot) => {
    snapshot.docChanges().forEach((change) => {
      if (change.type === 'added') {
        let data = change.doc.data();
        pc.addIceCandidate(new RTCIceCandidate(data));
      }
    });
  });
};