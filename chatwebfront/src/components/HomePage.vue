<template>
  <div class="container" id="container">
    <div class="form-container sign-up-container">
      <!-- 注册 -->
      <form @submit.prevent="register">
        <h1>Sign Up</h1>
        <div class="social-container">
          <a href="#" class="social"><i class="iconfont icon-qq"></i></a>
          <a href="#" class="social"><i class="iconfont icon-weixin"></i></a>
          <a href="#" class="social"><i class="iconfont icon-weibo-copy"></i></a>
          <a href="#" class="social"><i class="iconfont icon-github"></i></a>
        </div>
        <span id="register_text">You can choose one of the above methods to register your account </span>
        <input type="text"  placeholder="Username" v-model="registerForm.username" />
        <br>  
        <input type="password" placeholder="Password" v-model="registerForm.password" />
        <input type="email" placeholder="E-Mail" v-model="registerForm.email" />
        <button id="send_code">Send Verification code</button>
        <input type="email" placeholder="VERIFICATION CODE" v-model="registerForm.code" />
        <button type="submit">Sign Up</button>
      </form>
    </div>
    <div class="form-container sing-in-container">
      <!-- 登陆 -->
      <form @submit.prevent="login">
        <h1>User Login</h1>
        <div class="social-container">
          <a href="#" class="social"><i class="iconfont icon-qq"></i></a>
          <a href="#" class="social"><i class="iconfont icon-weixin"></i></a>
          <a href="#" class="social"><i class="iconfont icon-weibo-copy"></i></a>
          <a href="#" class="social"><i class="iconfont icon-github"></i></a>
        </div>
        <span id="loginresult">You can choose one of the above methods to log into your account!</span>
        <input type="text" placeholder="Username" v-model="loginForm.username" />
        <input type="password" placeholder="Password" v-model="loginForm.password" />
        <a href="#" id="errorText">Forget Password?</a>
        <button type="submit">Log In</button>
      </form>
    </div>
    <!-- 侧边栏内容 -->
    <div class="overlay-container">
      <div class="overlay">
        <div class="overlay-pannel overlay-left">
          <h1 id= "register_username">Already have an account?</h1>
          <p>Click here to log in, darling</p>
          <button class="ghost" id="signIn">Log In</button>
        </div>
        <div class="overlay-pannel overlay-right">
          <h1>Don't have an account?</h1>
          <p>Click here to register and create your own account</p>
          <button class="ghost" id="signUp">Sign Up</button>
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

        // 将响应文本解析为JSON
        const result = await response.json(); 
                // 更新并显示错误消息
                document.getElementById('loginresult').innerText = result.message;
                if(result.status==='error')
                  document.getElementById('loginresult').style.color = 'red'; // 可选：改变文字颜色
                else
                  document.getElementById('loginresult').style.color = 'blue'; // 可选：改变文字颜色
                this.showRegisterText();          
        if (result.status === 'success')
          this.$router.push('/chatroom');
        console.log(result);
      } catch (error) {
        console.error('登录失败:', error);
        this.showLoginErrorText();
      }
    },
    async register() {
      
      try {
        const response = await fetch('http://localhost:9176/register', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify(this.registerForm)
        });
        const result = await response.json();    
                document.getElementById('register_username').innerText = result.message;
                document.getElementById('register_username').style.color = 'pink'; // 可选：改变文字颜色

        // 处理后端响应
        if(result.status==='success')
          this.showRegisterText('Register Success!',0);       
        console.log(result);
      } catch (error) {
        console.error('注册失败:', error);
        this.showRegisterText('Register Failed',1);
      }
    },
    showLoginErrorText() {
      const errorText = document.getElementById('errorText');
      errorText.innerText = 'Login Failed';
      errorText.style.color = 'red';
    },
    showRegisterText(message, type) {
      const Text = document.getElementById('register_text');
      Text.innerText = message;
      if(type===1)
      Text.style.color = 'red';
      else
      Text.style.color = 'blue';
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