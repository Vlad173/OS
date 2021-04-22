#include <unistd.h>
#include <sstream>
#include <set>

#include "zmq_functions.h"
#include "topology.h"

int main() {
    topology network;
    std::vector<zmq::socket_t> branches;
    zmq::context_t context; 

    std::string cmd;
    while (std::cin >> cmd) {

        if (cmd == "create") {
            int node_id, parent_id;
            std::cin >> node_id >> parent_id;

            if (network.find(node_id) != -1) {
                std::cout << "Error: already exists" << std::endl;
            }
             else if (parent_id == -1) {
                pid_t pid = fork();
                if (pid < 0) {
                    perror("Can't create new process");
                    return -1;
                }
                if (pid == 0) {
                    if (execl("./counting", "./counting", std::to_string(node_id).c_str(), NULL) < 0) {
                        perror("Can't execute new process");
                        return -2;
                    } 
                }
                
                branches.emplace_back(context, ZMQ_REQ); 
                branches[branches.size() - 1].setsockopt(ZMQ_SNDTIMEO, 5000);
                bind(branches[branches.size() - 1], node_id);
                send_message(branches[branches.size() - 1], std::to_string(node_id) + "pid");
                
                std::string reply = receive_message(branches[branches.size() - 1]);
                std::cout << reply << std::endl;
                network.insert(node_id, parent_id);
            }
            else if (network.find(parent_id) == -1) {
                std::cout << "Error: parent not found" << std::endl;
            }
            else {
                int branch = network.find(parent_id);
                send_message(branches[branch], std::to_string(parent_id) + "create " + std::to_string(node_id));

                std::string reply = receive_message(branches[branch]);
                std::cout << reply << std::endl;
                network.insert(node_id, parent_id);
            }
        }
        else if (cmd == "exec") {
            int dest_id;
            std::string numbers;
            std::cin >> dest_id;
            std::getline(std::cin, numbers);
            int branch = network.find(dest_id);
            if (branch == -1) {
                std::cout << "ERROR: incorrect node id" << std::endl;
            }
            else {
                send_message(branches[branch], std::to_string(dest_id) + "exec " + numbers);
                std::string reply = receive_message(branches[branch]);
                std::cout << reply << std::endl;
            }
        }
        else if (cmd == "kill") {
            int id;
            std::cin >> id;
            int branch = network.find(id);
            if (branch == -1) {
                std::cout << "ERROR: incorrect node id" << std::endl;
            }
            else {
                bool is_first = (network.get_first_id(branch) == id);
                send_message(branches[branch], std::to_string(id) + " kill");

                std::string reply = receive_message(branches[branch]);
                std::cout << reply << std::endl;
                network.erase(id);
                if (is_first) {
                    unbind(branches[branch], id);
                    branches.erase(branches.begin() + branch);
                }
            }
        }
        else if (cmd == "heartbeat") {
            std::set<int> available_nodes;
            for (size_t i = 0; i < branches.size(); ++i) {
                int first_node_id = network.get_first_id(i);
                send_message(branches[i], std::to_string(first_node_id) + " heartbeat");
                
                std::string received_message = receive_message(branches[i]);
                std::istringstream reply(received_message);
                int node;
                while(reply >> node) {
                    available_nodes.insert(node);
                }
            }
            std::cout << "OK: ";
            if (available_nodes.empty()) {
                std::cout << "No available nodes" << std::endl;
            }
            else {
                for (auto v : available_nodes) {
                    std::cout << v << " ";
                }
                std::cout << std::endl;
            }
        }
        else if (cmd == "exit") {
            for (size_t i = 0; i < branches.size(); ++i) {
                int first_node_id = network.get_first_id(i);
                send_message(branches[i], std::to_string(first_node_id) + " kill");
                
                std::string reply = receive_message(branches[i]);
                if (reply != "OK") {
                    std::cout << reply << std::endl;
                }
                else {
                    unbind(branches[i], first_node_id);
                }
            }
            exit(0);
        }
        else {
            std::cout << "Incorrect cmd" << std::endl;
        }
    }
}