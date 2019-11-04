#include "OfflineSocket.h"


namespace osuCrypto {
    // Sending a vector of blocks to queue
    // void OfflineSocket::send(const std::vector<block> data, u64 size)
    // {
    //     auto size_block = size / sizeof(block);
    //     for (u64 i = 0; i < size_block; i++) {
    //         q_block.push_back(data[i]);
    //         std::cout << data[i];
    //     }
    // }

    // Sending data given by u8* by copying into a vector of blocks and then adding to queue
    void OfflineSocket::send(const u8* data, u64 size)
    {
        for (u64 i = 0; i < size; i++) {
            q_u8.push_back(data[i]);
        }
    }

    // Sending vector of blocks to queue
    void OfflineSocket::asyncSend(const std::vector<block> data)
    {
        send((u8*) data.data(), (u64) data.size() * sizeof(block));
    }

    void OfflineSocket::asyncSend(const std::vector<u8> data)
    {
        send((u8*) data.data(), (u64) data.size() * sizeof(u8));
    }

    // Sending gates to queue
    void OfflineSocket::asyncSend(const std::vector<GarbledGate<2>> gates)
    {
        for (u64 i = 0; i < gates.size(); i++) {
            q_gate.push_back(gates[i]);
        }
    }

    // Sending BitVector pointer to queue
    void OfflineSocket::asyncSend(std::unique_ptr<BitVector> data)
    {
        for (u64 i = 0; i < data->size(); i++) {
            q_bit.push_back((*data)[i]);
        }
    }

    // Make a copy of the data as a vector of blocks and then send to queue
    void OfflineSocket::asyncSendCopy(const u8* data, u64 size)
    {
        std::vector<u8> data_copy;
        for (int i = 0; i < size; i++) {
            data_copy.push_back(data[i]);
        }
        asyncSend(data_copy); 
    }

    // Receive for vector of blocks but given u8* instead
    void OfflineSocket::recv(u8* data, u64 size) {
        // int count = 0;
        // while (q_u8.empty()) {
        //     std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        //     count++;
        //     std::cerr << "WAITING" << std::endl;
        //     if (count > 1) {
        //         std::cout << "WAITING TOO LONG" << std::endl;
        //     }
        // }
        // std::cout << q_u8.size() << std::endl;
        for (u64 i = 0; i < size; i++) {
            data[i] = q_u8.front();
            q_u8.pop_front();
        }

        // std::cout << "START REC VALUES" << std::endl;

        // for (int i = 0; i < size/sizeof(block); i++) {
        //     std::cout << ((block*) data)[i] << std::endl;
        // }
        // std::cout << "END REC VALUES" << std::endl;
    }

    // Receive for vector of blocks
    void OfflineSocket::recv(std::vector<block>& data) {
        data.resize(16);
        std::cout << q_u8.size() << std::endl;
        recv((u8*) data.data(), 16 * sizeof(block));
    }

    // Receive for vector of blocks
    // void OfflineSocket::recv(std::vector<block>& data, u64 size) {
    //     data.resize(size);
    //     recv((u8*) data.data(), size * sizeof(block));
    // }

    void OfflineSocket::recv(std::vector<block>& gates, u64 size) {
        gates.resize(size*2);
        std::cout << "GATE VALUES" << std::endl;
        std::cout << q_gate.size() << std::endl;
        std::cout << size << std::endl;
        for (u64 i = 0; i < size*2; i+=2) {
            GarbledGate<2> gate_pair = q_gate.front();
            gates[i] = gate_pair.mGarbledTable[0];
            gates[i+1] = gate_pair.mGarbledTable[1];
            q_gate.pop_front();
        }
    }

    // Receive for BitVector
    void OfflineSocket::recv(BitVector& data) {
        for (u64 i = 0; i < data.size(); i++) {
            data[i] = q_bit.front();
            q_bit.pop_front();
        }
    }

};
