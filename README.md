# ChatWeb
Chat on web

## Usage:
### Backend setup
**1.Navigate to the backend directory and compile the HTTP server:**
```
cd backend
gcc httpserver.c -o hser
nodemon
```

**2.Compile the main server with necessary libraries:**
```
cd backend
// make sure you have installed these parts
gcc server.c -o cser -lssl -lcrypto -lpthread -liconv
```
### Frontend setup
**1.Navigate to the frontend directory, install dependencies, and run the development server:**
```
cd chatwebfront
npm install
npm run serve
npm run serve:8081
```
**2.To run additional instances of the server, modify package.json and run:**
```
npm run serve:8082
```
BTW, I don't recommend you to run too many servers concurrently, as each client connection increases the number of backend server threads. Then your cpu could, Boom! I'll consider to use thread pool to fix this.


## Updating.......
Chat server
Signaling server
Register/Sign up

Tools: mysql, vue, socket, c, Linux/Macos, Webrtc<br>

## Features to be developed:
Hashing the passwords when users register for an account. (Use salt to avoid mysql server attack)
Using webrtc to video chat with other clients

## Parts to be improve
**1.Thread pool** 
Emmm, this's a big issue, the thread will be created for each client's connection, obviously it's inefficient.
**2.VUE part** 
As I lack extensive experience with Vue, the frontend may not be as clean or efficient as it could be. I plan to learn more and improve this part over time.

### TBH, my knowledge in some areas is limited, so updates might be slow.

![Screenshot 2024-07-04 at 23.06.35](https://raw.githubusercontent.com/Hexlove2/images1/main/images/Screenshot%202024-07-04%20at%2023.06.35.png)