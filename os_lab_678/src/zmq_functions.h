#include <zmq.hpp>
#include <iostream>
 
const int MAIN_PORT = 4040;
 
void send_message(zmq::socket_t& socket, const std::string& msg) {
    //std::cout << "Start sending " << msg << std::endl;
    zmq::message_t message(msg.size());
    memcpy(message.data(), msg.c_str(), msg.size());
    socket.send(message);
    //std::cout << "Finished sending " << msg << std::endl;
}
 
std::string receive_message(zmq::socket_t& socket) {
    //std::cout << "Start receiving" << std::endl;
    zmq::message_t message;
    bool chars_read;
    try {
        chars_read = socket.recv(&message);
    }
    catch (...) {
        chars_read = false;
    }
    if (chars_read == 0) {
        return "Error: node is unavailable [zmq_func]";
    }
    std::string received_msg(static_cast<char*>(message.data()), message.size());
    //std::cout << "Received " << received_msg << std::endl;
    return received_msg;
}
 
void connect(zmq::socket_t& socket, int id) {
    std::string address = "tcp://127.0.0.1:" + std::to_string(MAIN_PORT + id);
    socket.connect(address);
    //std::cout << "Socket connected to " << address << std::endl;
}

void disconnect(zmq::socket_t& socket, int id) {
    std::string address = "tcp://127.0.0.1:" + std::to_string(MAIN_PORT + id);
    socket.disconnect(address);
    //std::cout << "Socket disconnected from " << address << std::endl;
}

void bind(zmq::socket_t& socket, int id) {
    std::string address = "tcp://127.0.0.1:" + std::to_string(MAIN_PORT + id);
    socket.bind(address);
    //std::cout << "Socket binded to " << address << std::endl;
}

void unbind(zmq::socket_t& socket, int id) {
    std::string address = "tcp://127.0.0.1:" + std::to_string(MAIN_PORT + id);
    socket.unbind(address);
    //std::cout << "Socket unbinded from " << address << std::endl;
}