#pragma once

#include <vector>
#include <complex>
#include <uhd/stream.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <boost/atomic.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include "gpsgen.hpp"

namespace blackstar
{
    class UHDTxSamples final
    {
    public:
        UHDTxSamples() = default;
        ~UHDTxSamples() = default;
                
        void build(boost::lockfree::spsc_queue<iq_buff_t*, boost::lockfree::capacity<16>> *queue, int argc, char *argv[]);
        void enableDebugMessages(bool enable = true);
        bool initialiseSDR();
        void start();
        void quit();
        void sendThenQuit();
        
    private:        
        static const size_t k_startThreshold { 4 }; // Wait for this number of buffers to be available before starting send        
        uhd::usrp::multi_usrp::sptr m_usrp { nullptr };
        uhd::tx_streamer::sptr m_tx_stream { nullptr };
        bool sendSamples(std::vector<std::complex<short>> *iqBuffer);
        boost::lockfree::spsc_queue<iq_buff_t*, boost::lockfree::capacity<16>> *m_queue { nullptr };        
        boost::atomic<bool> m_sendThenQuit { false };
        boost::atomic<bool> m_done { false };
        boost::atomic<bool> m_debugMessages { false };
        double m_SDRGain { -100.0 };
    };
}