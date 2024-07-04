# 参考资料与代码

## 1.WebRtc部分

### 1.前端vue3中的WebRtc使用

```vue
// 来自csdn
//https://blog.csdn.net/qq_62835094/article/details/136094489?utm_medium=distribute.pc_relevant.none-task-blog-2~default~baidujs_baidulandingword~default-0-136094489-blog-136538227.235^v43^pc_blog_bottom_relevance_base8&spm=1001.2101.3001.4242.1&utm_relevant_index=3

<script setup lang='ts'>
// 导入必要的模块和常量
import { Message, SIGNAL_TYPE_ANSWER, SIGNAL_TYPE_CANDIDATE, SIGNAL_TYPE_JOIN, SIGNAL_TYPE_NEW_PEER, SIGNAL_TYPE_OFFER, SIGNAL_TYPE_RESP_JOIN } from "@/api/webrtc";
import { use } from "echarts";
import { useId } from "element-plus";
import { off } from "process";
import { onMounted } from "vue";
import { ref } from "vue";

// 定义本地和远程视频元素和流的响应式引用
const localVideo = ref<HTMLVideoElement>();
const localStream = ref<MediaStream>();

const remoteVideo = ref<HTMLVideoElement>();
const remoteStream = ref<MediaStream>();

// 定义RTCPeerConnection的响应式引用
const pc = ref<RTCPeerConnection>();

// 为本地用户生成一个唯一的userId，并定义远程用户ID的响应式引用
const userId = ref<string>(Math.random().toString(36).substr(2));
const remoteUserId = ref<string>();

// 创建一个新的WebSocket连接
const ws = new WebSocket("ws://localhost:1000/ws2/" + userId.value);

// WebSocket连接成功打开时的事件处理程序
ws.onopen = (ev: Event) => {
  console.log("连接成功 userId = " + userId.value);
}

// WebSocket收到消息时的事件处理程序
ws.onmessage = (ev: MessageEvent) => {
  const message : Message = JSON.parse(ev.data);
  if (message.cmd === SIGNAL_TYPE_NEW_PEER) {
    handleRemoteNewPeer(message);
  } else if (message.cmd === SIGNAL_TYPE_RESP_JOIN) {
    handleResponseJoin(message); 
  } else if (message.cmd === SIGNAL_TYPE_OFFER) {
    handleRemoteOffer(message);
  } else if (message.cmd === SIGNAL_TYPE_ANSWER) {
    handleRemoteAnswer(message);
  } else if (message.cmd === SIGNAL_TYPE_CANDIDATE) {
    handleRemoteCandidate(message);
  }
}

// WebSocket连接关闭时的事件处理程序
ws.onclose = (ev) => {
  console.log("连接关闭 userId = " + userId.value);
}

// 处理对端发送的candidate
const handleRemoteCandidate = (message : Message) => {
  console.log("handleRemoteCandidate...");
  pc.value.addIceCandidate(message.data);
}

// 保存远端answer
const handleRemoteAnswer = (message : Message) => {
  console.log("handleRemoteAnswer...");
  pc.value.setRemoteDescription(message.data);
}

// 保存远端offer, 创建answer
const handleRemoteOffer = async (message : Message) => {
  console.log("handleRemoteOffer...");
  // 保存远端offer
  pc.value.setRemoteDescription(message.data);
  // 创建自己的offer（answer）
  const answer = await pc.value.createAnswer();
  // 保存自己的answer
  pc.value.setLocalDescription(answer);
  // 发送answer
  const answerMsg = new Message();
  answerMsg.cmd = SIGNAL_TYPE_ANSWER;
  answerMsg.userId = userId.value;
  answerMsg.remoteUserId = remoteUserId.value;
  answerMsg.data = answer;
  ws.send(JSON.stringify(answerMsg));
}

// 保存remoteUserId
const handleResponseJoin = (message : Message) => {
  console.log("handleResponseJoin...");
  remoteUserId.value = message.remoteUserId;
}

// 创建offer，设置本地offer并且发送给对端 
const handleRemoteNewPeer = async (message : Message) => {
  console.log("handleRemoteNewPeer...");
  // 保存remoteUserId
  remoteUserId.value = message.remoteUserId;
  // 创建offer
  const offer = await pc.value.createOffer();
  // 保存本地offer
  pc.value.setLocalDescription(offer);
  // 发送offer
  const offerMsg = new Message();
  offerMsg.cmd = SIGNAL_TYPE_OFFER;
  offerMsg.data = offer;
  offerMsg.remoteUserId = remoteUserId.value;
  offerMsg.userId = userId.value;
  ws.send(JSON.stringify(offerMsg));
}

// 组件挂载时执行的函数
onMounted(() => {
  localVideo.value = document.querySelector("#localVideo"); 
  remoteVideo.value = document.querySelector("#remoteVideo"); 
})

// 加入房间
const join = async () => {
  console.log("join...");
  // 获取本地视频流
  localStream.value = await navigator.mediaDevices.getUserMedia({
    audio: true,
    video: true
  });
  localVideo.value.srcObject = localStream.value;

  // 创建peerConnection
  pc.value = new RTCPeerConnection();
  // 将本地流的控制权交给pc
  localStream.value.getTracks().forEach(track => { pc.value.addTrack(track, localStream.value) });
  // 设置pc的onicecandidate和ontrack事件处理程序
  pc.value.onicecandidate = (event : RTCPeerConnectionIceEvent) => {
    if (event.candidate) {
      // 发送candidate
      const candidateMsg = new Message();
      candidateMsg.cmd = SIGNAL_TYPE_CANDIDATE;
      candidateMsg.data = event.candidate;
      candidateMsg.userId = userId.value;
      candidateMsg.remoteUserId = remoteUserId.value;
      ws.send(JSON.stringify(candidateMsg));
    } else {
      console.log("there is no candidate availiable...");
    }
  }
  pc.value.ontrack = (event : RTCTrackEvent) => {
    // 保存远端视频流
    remoteStream.value = event.streams[0];
    remoteVideo.value.srcObject = event.streams[0];
  }

  // 加入房间
  const message = new Message();
  message.userId = userId.value;
  message.cmd = SIGNAL_TYPE_JOIN;
  ws.send(JSON.stringify(message));
}
</script>

<template>
  <el-button @click="join">加入</el-button>
  <div id="videos">
    <video id="localVideo" autoplay muted playsinline>本地窗口</video>
    <video id="remoteVideo" autoplay playsinline>远端窗口</video>
  </div>
</template>

<style lang='scss' scoped>

</style>
```

### 2.后端Java中的信令服务器搭建

```java
package com.fgbg.webrtc.websocket;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fgbg.webrtc.model.Client;
import com.fgbg.webrtc.model.Message;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Component;

import javax.websocket.OnClose;
import javax.websocket.OnMessage;
import javax.websocket.OnOpen;
import javax.websocket.Session;
import javax.websocket.server.PathParam;
import javax.websocket.server.ServerEndpoint;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

import static com.fgbg.webrtc.model.Constant.*;

@Component
@Slf4j
@ServerEndpoint("/ws2/{userId}")
public class WebSocket2 {

    // 与某个客户端的连接会话，需要通过它来给客户端发送数据
    private Client client;

    // 存储用户的映射表，key是用户ID，value是用户对应的Client对象
    private static Map<String, Client> clientMap = new ConcurrentHashMap<>();

    // 存储房间的映射表，key是房间ID，value是房间中的用户ID集合
    private static Map<String, Set<String>> roomMap = new ConcurrentHashMap<>();

    // 初始化一个房间，房间ID为1
    static {
        roomMap.put("1", new HashSet<String>());
    }

    private ObjectMapper objectMapper = new ObjectMapper();

    // 当一个新用户连接时触发
    @OnOpen
    public void onOpen(Session session, @PathParam(value="userId") String userId) {
        log.info("userId = " + userId + " 加入房间1");
        Client client = new Client();
        client.setRoomId("1");
        client.setSession(session);
        client.setUserId(userId);
        this.client = client;
        clientMap.put(userId, client);
    }

    // 当用户断开连接时触发
    @OnClose
    public void onClose() {
        String userId = client.getUserId();
        clientMap.remove(userId);
        roomMap.get("1").remove(userId);
        log.info("userId = " + userId + " 退出房间1");
    }

    // 当服务器收到消息时触发
    @OnMessage
    public void onMessage(String message) throws JsonProcessingException {
        Message data = objectMapper.readValue(message, Message.class);
        if (data.getCmd().equals(SIGNAL_TYPE_JOIN)) {
            handleJoin(data);
        } else if (data.getCmd().equals(SIGNAL_TYPE_OFFER)) {
            handleOffer(data);
        } else if (data.getCmd().equals(SIGNAL_TYPE_ANSWER)) {
            handleAnswer(data);
        } else if (data.getCmd().equals(SIGNAL_TYPE_CANDIDATE)) {
            handleCandidate(data);
        }
    }

    // 处理candidate消息
    private void handleCandidate(Message message) throws JsonProcessingException {
        String remoteUserId = message.getRemoteUserId();
        sendMsgByUserId(message, remoteUserId);
    }

    // 处理answer消息
    private void handleAnswer(Message message) throws JsonProcessingException {
        String remoteUserId = message.getRemoteUserId();
        sendMsgByUserId(message, remoteUserId);
    }

    // 处理offer消息
    private void handleOffer(Message message) throws JsonProcessingException {
        String remoteUserId = message.getRemoteUserId();
        sendMsgByUserId(message, remoteUserId);
    }

    // 处理用户加入房间逻辑
    private void handleJoin(Message message) throws JsonProcessingException {
        String userId = message.getUserId();
        String roomId = message.getRoomId();
        clientMap.get(userId).setRoomId(roomId);
        roomMap.get(roomId).add(userId);

        int size = roomMap.get(roomId).size();

        if (size == 1) {
            return; // 房间中只有一个用户，什么都不做
        } else if (size == 2) {
            String remoteUserId = null;
            for (String id : roomMap.get(roomId)) {
                if (!id.equals(userId)) {
                    remoteUserId = id;
                    break;
                }
            }
            // 通知第一个用户有新用户加入
            Message newPeerMsg = new Message();
            newPeerMsg.setCmd(SIGNAL_TYPE_NEW_PEER);
            newPeerMsg.setRoomId(roomId);
            newPeerMsg.setUserId(remoteUserId);
            newPeerMsg.setRemoteUserId(userId);
            sendMsgByUserId(newPeerMsg, remoteUserId);

            // 通知第二个用户加入成功
            Message respJoinMsg = new Message();
            respJoinMsg.setCmd(SIGNAL_TYPE_RESP_JOIN);
            respJoinMsg.setRoomId(roomId);
            respJoinMsg.setUserId(userId);
            respJoinMsg.setRemoteUserId(remoteUserId);
            sendMsgByUserId(respJoinMsg, userId);
        } else if (size > 2) {
            log.error("人数超过2人, 拒绝加入");
        }
    }

    // 根据用户ID发送消息
    private void sendMsgByUserId(Message msg, String remoteId) throws JsonProcessingException {
        Client client = clientMap.get(remoteId);
        client.getSession().getAsyncRemote().sendText(objectMapper.writeValueAsString(msg));
    }
}
```

### 3.JS实现信令服务器，包含控制html元素
```javascript
//from github
//https://github.com/fireship-io/webrtc-firebase-demo
// 导入样式文件，定义页面的样式
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
```