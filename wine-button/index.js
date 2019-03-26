const opn = require('opn');
const app = require('express')();
const http = require('http').Server(app);
const io = require('socket.io')(http);

io.on('connection', function (socket) {
    socket.on('wine', (data) => {
        console.log(`wine-bot says: ${data}`);
        opn('https://beyerskloof.co.za/online-shop/?orderby=price&add-to-cart=139');
    });
});

http.listen(3000, function () {
    console.log('listening on *:3000');
});
