<template>
  <div class="chat-container-chatroom">
    <h2>Welcome to the Chat Room</h2>

    <div class="video-container">
      <video ref="localVideo" autoplay muted></video>
      <video ref="remoteVideo" autoplay></video>
    </div>

    <div class="messages-chatroom">
      <div v-for="(message, index) in messages" :key="index">
        <span class="username-chatroom" v-html="message.username"></span>
        <span class="message-content-chatroom" v-html="message.content"></span>
      </div>
    </div>

    <div class="input-container-chatroom">
      <input v-model="newMessage" placeholder="Type your message here..." @keypress.enter="sendMessage" @input="handleInput" >
      <button @click="sendMessage">Send</button>
    </div>

    <div class="video-buttons">
      <button id="videostart" @click="startVideo" >StartVideo</button>
      <button id="videohangup" @click="stopVideo">Hangup</button>
    </div>

  </div>
</template>



<script>

//**************************************************************************************************************************/
export default {
  name: 'ChatRoom',
  props: ['username'],
  data() {
    return {
      newMessage: '',
      messages: [],
      socket: null,
      messageQueue: [] ,
      localStream:null,
      remoteStream:null,
      peerConnection:null,
    };
  },
  created(){
    this.connectWebSocket();
    window.addEventListener('beforeunload', function () {
    if (this.socket && this.socket.readyState === WebSocket.OPEN) {
        this.socket.close();
    }
    
});
  },
  
  methods: {
    connectWebSocket(){
      this.socket = new WebSocket('ws://192.168.31.137:7777');

      this.socket.onopen = () => {
          console.log('WebSocket connection opened');
          //this.socket.send('Hello!');
          console.log('已发送hello消息');
          /*
          while (this.messageQueue.length > 0) {
          const message = this.messageQueue.shift();
          this.socket.send(JSON.stringify(message));
          
          }*/
      };

      this.socket.onmessage = (event) =>{
        const data= JSON.parse(event.data);
        


        
          this.messages.push(data);
          this.$nextTick(()=>{
            this.scrollToBottom();
          });
        };
        
      },
    
      sendMessage() {
      if (this.newMessage.trim() !== '') {
        const message = {
          username: `<span style="color: blue;">[${this.username}]</span>`,
          content: this.newMessage.trim() // Trim whitespace properly
        };
        this.messages.push(message);

        console.log('Prepared message to send:', message); // Log the message

        if (this.socket.readyState === WebSocket.OPEN) {
          try {
            const jsonMessage = JSON.stringify(message);
            console.log('JSON message to send:', jsonMessage); // Log the JSON message
            this.socket.send(jsonMessage);
            console.log('Message sent:', message);
          } catch (error) {
            console.error('Failed to send message:', error);
          }
        } else {
          console.error('WebSocket is not open. Ready state is:', this.socket.readyState);
        }

        this.newMessage = '';
        this.$nextTick(() => {
          this.scrollToBottom();
        });
      } else {
        console.log('New message is empty or only whitespace'); // Log if newMessage is empty or whitespace
      }
    },
    scrollToBottom() {
      const container = this.$el.querySelector('.messages-chatroom');
      container.scrollTop = container.scrollHeight;
    },

    startVideo() {
      navigator.mediaDevices.getUserMedia({ video: true, audio: true })
        .then(stream => {
          this.localStream = stream;
          this.$refs.localVideo.srcObject = stream;
          stream.getTracks().forEach(track => {
            this.peerConnection.addTrack(track, stream);
          });
        })
        .catch(error => console.error('Error accessing media devices.', error));
    },
    stopVideo() {
      if (this.localStream) {
        this.localStream.getTracks().forEach(track => track.stop());
        this.localStream = null;
      }
    },


    toggleButton(button) {
      const buttonElement = button === 'start' ? this.$el.querySelector('.video-buttons button:first-child') : this.$el.querySelector('.video-buttons button:last-child');
      buttonElement.classList.toggle('toggled');
    }
  }
};
</script>

<style scoped>
.chat-container-chatroom {
  width: 600px;
  margin: 0 auto;
  border: 1px solid #ccc;
  border-radius: 5px;
  padding: 20px;
}
.video-container {
  display: flex;
  justify-content: space-between;
  width: 100%;
  max-width: 800px;
  margin-bottom: 20px;
}
video {
  width: 48%;
  border: 1px solid #ccc;
  border-radius: 5px;
  object-fit: cover;
}
.messages-chatroom {
  height: 400px;
  overflow-y: auto;
  border: 1px solid #ccc;
  padding: 10px;
  margin-bottom: 10px;
}
.input-container-chatroom {
  display: flex;
  justify-content: space-between;
}
input {
  flex: 1;
  padding: 10px;
  margin-right: 10px;
}
button {
  padding: 10px;
  background: green;
  color: white;
  border: none;
  border-radius: 5px;
}
.username-chatroom {
  font-weight: bold;
}
.message-content-chatroom {
  margin-left: 5px;
}
button.active {
  background: red;
}
button.toggled {
  background: rgb(146, 145, 181);
}
</style>