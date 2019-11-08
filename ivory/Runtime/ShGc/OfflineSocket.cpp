#include "OfflineSocket.h"


namespace osuCrypto {

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
        for (u64 i = 0; i < size; i++) {
            data[i] = q_u8.front();
            q_u8.pop_front();
        }
    }

    // Receive for vector of blocks
    void OfflineSocket::recv(std::vector<block>& data) {
        // 16 comes from the bitcount variable in the main program
        data.resize(16);
        recv((u8*) data.data(), 16 * sizeof(block));
    }

    // Receive for vector of blocks
    // void OfflineSocket::recv(std::vector<block>& data, u64 size) {
    //     data.resize(size);
    //     recv((u8*) data.data(), size * sizeof(block));
    // }

    void OfflineSocket::recv(std::vector<block>& gates, u64 size) {
        gates.resize(size*2);
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

    std::deque<block> OfflineSocket::getQGate() {
        // Serialize by just sequentially storing the 2 garbled tables
        std::deque<block> serialized_q_gate;
        while (q_gate.size()) {
            GarbledGate<2> gate_pair = q_gate.front();
            serialized_q_gate.push_back(gate_pair.mGarbledTable[0]);
            serialized_q_gate.push_back(gate_pair.mGarbledTable[1]);
            q_gate.pop_front();
        }
        return serialized_q_gate;
    }

    std::deque<u8> OfflineSocket::getQu8() {
        return q_u8;
    }

    std::deque<BitReference> OfflineSocket::getQBit() {
        return q_bit;
    }

    void OfflineSocket::setQGate(std::deque<block> q) {
        while (q.size()) {
            block garbleTable_1 = q.front();
            q.pop_front();
            block garbleTable_2 = q.front();
            GarbledGate<2> gate_pair = {garbleTable_1, garbleTable_2};
            q_gate.push_back(gate_pair);
            q.pop_front();
        }
    }

    void OfflineSocket::setQu8(std::deque<u8> q) {
        q_u8 = q;
    }

    void OfflineSocket::setQBit(std::deque<BitReference> q) {
        q_bit = q;
    }

};
