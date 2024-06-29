import { createRouter, createWebHistory } from 'vue-router'
import HomePage from '../components/HomePage.vue'
import ChatRoom from '../components/ChatRoom.vue'

const routes = [
  { path: '/', name: HomePage, component: HomePage },   // 默认打开 HomePage 组件
  { path: '/chatroom', component: ChatRoom } // ChatRoom 路径指向 ChatRoom 组件
]

const router = createRouter({
  history: createWebHistory(),
  routes
})

export default router