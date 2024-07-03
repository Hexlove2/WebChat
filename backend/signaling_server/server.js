const express = require('express');
const http = require('http');
const socketIO = require('socket.io');
const cors = require('cors');

const app = express();

// 配置 CORS
app.use(cors({
    origin: '*', // 允许所有源访问，可以根据需求更改
    methods: ['GET', 'POST'],
    allowedHeaders: ['Content-Type']
}));

const server = http.createServer(app);
const io = socketIO(server, {
    cors: {
        origin: '*', // 允许所有源访问，可以根据需求更改
        methods: ['GET', 'POST']
    }
});

io.on('connection', (socket) => {
    console.log('a user connected');

    socket.on('offer', (offer) => {
        socket.broadcast.emit('offer', offer);
    });

    socket.on('answer', (answer) => {
        socket.broadcast.emit('answer', answer);
    });

    socket.on('candidate', (candidate) => {
        socket.broadcast.emit('candidate', candidate);
    });

    socket.on('disconnect', () => {
        console.log('user disconnected');
    });
});

server.listen(3000, () => {
    console.log('Server is running on port 3000');
});