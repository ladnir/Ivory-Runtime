#include <cryptoTools/Common/Defines.h>
#include <deque>
#include <cryptoTools/Circuit/BetaCircuit.h>
#include <cryptoTools/Common/BitVector.h>


namespace osuCrypto {
    class OfflineSocket {
        public:
            //std::deque<block> q_block;
            std::deque<GarbledGate<2>> q_gate;
            std::deque<BitReference> q_bit;
            std::deque<u8> q_u8;

        OfflineSocket() {}

        ~OfflineSocket() {}

        // void send(const std::vector<block> data, u64 size);

        void send(const u8* data, u64 size);

        void asyncSend(const std::vector<u8> data);

        void asyncSend(const std::vector<block> data);

        void asyncSend(const std::vector<GarbledGate<2>> gates);

        void asyncSend(std::unique_ptr<BitVector> data);

        void asyncSendCopy(const u8* data, u64 size); 

        void recv(u8* data, u64 size);

        void recv(std::vector<block>& data);

        void recv(std::vector<block>& gates, u64 size);

        void recv(BitVector& data);

        std::deque<GarbledGate<2>> getQGate();

        std::deque<u8> getQu8();

        std::deque<BitReference> getQBit();

        void setQGate(std::deque<GarbledGate<2>> q);

        void setQu8(std::deque<u8> q);

        void setQBit(std::deque<BitReference> q);

        // std::deque<block>& get_q() {
        //     return q;
        // }

        // void set_other_q(std::deque<block>* other_q) {
        //     q_other = other_q;
        // }
    
    };
}