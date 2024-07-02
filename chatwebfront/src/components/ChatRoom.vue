<template>
  <div class="chat-container-chatroom">
    <h2>Welcome to the Chat Room</h2>
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
  </div>
</template>

<script>
export default {
  name: 'ChatRoom',
  props: ['username'],
  data() {
    return {
      newMessage: '',
      messages: []
    };
  },
  methods: {
    sendMessage() {
      if (this.newMessage.trim() !== '') {
        const message = {
          username: `<span style="color: blue;">[${this.username}]</span>`,
          content: this.newMessage
        };
        this.messages.push(message);
        this.newMessage = '';
        this.$nextTick(() => {
          this.scrollToBottom();
        });
      }
    },
    scrollToBottom() {
      const container = this.$el.querySelector('.messages-chatroom');
      container.scrollTop = container.scrollHeight;
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
</style>