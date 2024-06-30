<template>
  <div class="container" id="container">
    <div class="form-container sign-up-container">
      <!-- 注册 -->
      <form @submit.prevent="register">
        <h1>用户注册</h1>
        <div class="social-container">
          <a href="#" class="social"><i class="iconfont icon-qq"></i></a>
          <a href="#" class="social"><i class="iconfont icon-weixin"></i></a>
          <a href="#" class="social"><i class="iconfont icon-weibo-copy"></i></a>
          <a href="#" class="social"><i class="iconfont icon-github"></i></a>
        </div>
        <span>您可以选择以上几种方式注册一个您的账户!</span>
        <input type="text" placeholder="用户名" v-model="registerForm.username" />
        <input type="password" placeholder="密码" v-model="registerForm.password" />
        <input type="email" placeholder="邮箱" v-model="registerForm.email" />
        <button id="send_code">发送验证码</button>
        <input type="email" placeholder="验证码" v-model="registerForm.code" />
        <button type="submit">注册</button>
      </form>
    </div>
    <div class="form-container sing-in-container">
      <!-- 登陆 -->
      <form @submit.prevent="login">
        <h1>用户登陆</h1>
        <div class="social-container">
          <a href="#" class="social"><i class="iconfont icon-qq"></i></a>
          <a href="#" class="social"><i class="iconfont icon-weixin"></i></a>
          <a href="#" class="social"><i class="iconfont icon-weibo-copy"></i></a>
          <a href="#" class="social"><i class="iconfont icon-github"></i></a>
        </div>
        <span>您可以选择以上几种方式登陆您的账户!</span>
        <input type="text" placeholder="用户名" v-model="loginForm.username" />
        <input type="password" placeholder="密码" v-model="loginForm.password" />
        <a href="#" id="errorText">忘记密码?</a>
        <button type="submit">登陆</button>
      </form>
    </div>
    <!-- 侧边栏内容 -->
    <div class="overlay-container">
      <div class="overlay">
        <div class="overlay-pannel overlay-left">
          <h1>已有账号?</h1>
          <p>亲爱的快快点我去进行登陆吧。</p>
          <button class="ghost" id="signIn">登陆</button>
        </div>
        <div class="overlay-pannel overlay-right">
          <h1>没有帐号？</h1>
          <p>点击注册去注册一个属于你的账号吧。</p>
          <button class="ghost" id="signUp">注册</button>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
export default {
  name: 'HomePage',
  data() {
    return {
      loginForm: {
        username: '',
        password: ''
      },
      registerForm: {
        username: '',
        password: '',
        email: '',
        code: ''
      }
    };
  },
  methods: {
    async login() {
      try {
        console.log('waiting0...');
        const response = await fetch('http://localhost:9176/login', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify(this.loginForm)
        });
        // 打印响应的状态码和内容类型
        console.log('响应状态码:', response.status);
        console.log('响应内容类型:', response.headers.get('content-type'));

        // 检查响应内容
        const text = await response.text();
        console.log('响应文本:', text);

        // 将响应文本解析为JSON
        const result = JSON.parse(text);
        if (result.status === 'success')
          this.$router.push('/chatroom');
        console.log(result);
      } catch (error) {
        console.error('登录失败:', error);
        this.showErrorText();
      }
    },
    async register() {
      try {
        const response = await fetch('http://your-backend-url/register', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify(this.registerForm)
        });
        const result = await response.json();
        // 处理后端响应
        console.log(result);
      } catch (error) {
        console.error('注册失败:', error);
      }
    },
    showErrorText() {
      const errorText = document.getElementById('errorText');
      errorText.innerText = '登录失败';
      errorText.style.color = 'red';
    }
  },
  mounted() {
    const container = document.querySelector('#container');
    const signInButton = document.querySelector('#signIn');
    const signUpButton = document.querySelector('#signUp');

    signUpButton.addEventListener('click',() => container.classList.add('right-panel-active'))
    signInButton.addEventListener('click',() => container.classList.remove('right-panel-active'))
  }
};
</script>

<style>
@import '../assets/style.css';
@import '../assets/iconfont/iconfont.css';
</style>