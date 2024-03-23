//
// Copyright 2011-2012,2014 Ettus Research LLC
// Copyright 2018 Ettus Research, a National Instruments Company
//
// SPDX-License-Identifier: GPL-3.0-or-later
//

#include <uhd/types/tune_request.hpp>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/utils/thread.hpp>
#include <boost/format.hpp>
#include <chrono>
#include <complex>
#include <csignal>
#include <fstream>
#include <iostream>
#include <thread>
#include "uhd_tx_samples.hpp"

namespace blackstar
{
    
    void UHDTxSamples::build(boost::lockfree::spsc_queue<iq_buff_t*, boost::lockfree::capacity<16>> *queue, int argc, char *argv[])
    {
        m_queue = queue;
        int opt;
        // Command line options shared between UHD and GPSGen objects; validate based on all options
        while ((opt = getopt(argc, argv, "G:e:u:x:g:c:l:s:b:T:t:d:iv")) != -1)
        {
            switch (opt)
            {
            case 'G': 
                {
                    errno = 0;
                    char *pEnd { nullptr };                    
                    double gain { std::strtod(optarg, &pEnd) };
                    
                    if (*pEnd == '\0' &&  // Check that we consumed the entire string
                        errno == 0 )      // Check that there was no overflow or underflow
                    {
                        m_SDRGain = gain;
                    }                    
                }
                break;
            }
        }
    }
    
    void UHDTxSamples::enableDebugMessages(bool enable)
    {
        m_debugMessages = enable;
    }
    
    bool UHDTxSamples::sendSamples(std::vector<std::complex<short>> *iqBuffer)
    {
        bool ok { false };
        if (m_tx_stream != nullptr)
        {
            uhd::tx_metadata_t md;
            md.start_of_burst = false;
            md.end_of_burst   = false;
            
            size_t num_tx_samps { iqBuffer->size() };
            if (m_debugMessages)
            {
                std::cout << "UHD sending " << num_tx_samps << " samples" << std::endl;
            }
            const size_t samples_sent { m_tx_stream->send(&(iqBuffer->front()), num_tx_samps, md) };
            if (samples_sent == num_tx_samps)
            {
                ok = true;
            }
            else
            {
                UHD_LOG_ERROR("TX-STREAM",
                              "The tx_stream timed out sending " << num_tx_samps << " samples ("
                                                                 << samples_sent << " sent).");
            }
        }
        return ok;
    }

    bool UHDTxSamples::initialiseSDR()
    {
        bool ok { true };
        std::string args {};
        std::string ref { "external" };
        std::string wirefmt { "sc16" };
        std::string channel { "0" };
        double rate { 2.5e6 };
        double freq { 1575.42e6 };
        double lo_offset { 0.0 };
        
        // create a usrp device
        std::cout << std::endl;
        std::cout << boost::format("Creating the usrp device with: %s...") % args
                  << std::endl;
        m_usrp = uhd::usrp::multi_usrp::make(args);

        m_usrp->set_clock_source(ref);

        std::cout << boost::format("Using Device: %s") % m_usrp->get_pp_string() << std::endl;
        
        std::cout << boost::format("Setting TX Rate: %f Msps...") % (rate / 1e6) << std::endl;
        m_usrp->set_tx_rate(rate);
        std::cout << boost::format("Actual TX Rate: %f Msps...") % (m_usrp->get_tx_rate() / 1e6)
                  << std::endl
                  << std::endl;
        std::cout << boost::format("Setting TX Freq: %f MHz...") % (freq / 1e6) << std::endl;
        std::cout << boost::format("Setting TX LO Offset: %f MHz...") % (lo_offset / 1e6)
                  << std::endl;
        uhd::tune_request_t tune_request;
        tune_request = uhd::tune_request_t(freq, lo_offset);    
        m_usrp->set_tx_freq(tune_request);
        std::cout << boost::format("Actual TX Freq: %f MHz...") % (m_usrp->get_tx_freq() / 1e6)
                  << std::endl
                  << std::endl;

        // set the rf gain
        std::cout << boost::format("Setting TX Gain: %f dB...") % m_SDRGain << std::endl;
        m_usrp->set_tx_gain(m_SDRGain);
        std::cout << boost::format("Actual TX Gain: %f dB...") % m_usrp->get_tx_gain()
                  << std::endl
                  << std::endl;

        // allow for some setup time:
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Check Ref and LO Lock detect
        std::vector<std::string> sensor_names;
        sensor_names = m_usrp->get_tx_sensor_names(0);
        if (std::find(sensor_names.begin(), sensor_names.end(), "lo_locked")
            != sensor_names.end()) {
            uhd::sensor_value_t lo_locked = m_usrp->get_tx_sensor("lo_locked", 0);
            std::cout << boost::format("Checking TX: %s ...") % lo_locked.to_pp_string()
                      << std::endl;
            UHD_ASSERT_THROW(lo_locked.to_bool());
        }
        sensor_names = m_usrp->get_mboard_sensor_names(0);
        if ((ref == "mimo")
            and (std::find(sensor_names.begin(), sensor_names.end(), "mimo_locked")
                 != sensor_names.end())) {
            uhd::sensor_value_t mimo_locked = m_usrp->get_mboard_sensor("mimo_locked", 0);
            std::cout << boost::format("Checking TX: %s ...") % mimo_locked.to_pp_string()
                      << std::endl;
            UHD_ASSERT_THROW(mimo_locked.to_bool());
        }
        if ((ref == "external")
            and (std::find(sensor_names.begin(), sensor_names.end(), "ref_locked")
                 != sensor_names.end())) {
            uhd::sensor_value_t ref_locked = m_usrp->get_mboard_sensor("ref_locked", 0);
            std::cout << boost::format("Checking TX: %s ...") % ref_locked.to_pp_string()
                      << std::endl;
            UHD_ASSERT_THROW(ref_locked.to_bool());
        }    

        // create a transmit streamer
        std::string cpu_format { "sc16" };
        std::vector<size_t> channel_nums;
        uhd::stream_args_t stream_args(cpu_format, wirefmt);
        channel_nums.push_back(boost::lexical_cast<size_t>(channel));
        stream_args.channels             = channel_nums;
        m_tx_stream = m_usrp->get_tx_stream(stream_args);

        // finished
        std::cout << std::endl << "Transmit streamer ready" << std::endl << std::endl;

        return ok;
    }
    
    void UHDTxSamples::start()
    {
        bool startedSending { false };
        
        iq_buff_t *iqBuffer { nullptr };
        while (!m_done)
        {
            // Wait for some buffers to build up before starting send
            if (!startedSending && (m_queue->read_available() < k_startThreshold))
            {
                continue;
            }
            
            // Thread running - unload buffers and transmit them
            while ((!m_done || m_sendThenQuit) && m_queue->pop(iqBuffer))
            {
                startedSending = true;
                sendSamples(iqBuffer);
                delete iqBuffer;
                if (m_debugMessages)
                {
                    std::cout << "Read available: " << m_queue->read_available() << std::endl;
                }
            }
            
            // If we didn't get a buffer and we had started sending then there has been an underflow
            // (note - ignoring underflow when emptying buffers after m_done is asserted)
            if (!m_done && startedSending)
            {
                if (m_debugMessages)
                {
                    std::cerr << "ERROR: buffer underflow" << std::endl;
                }
            }
        }
        
        // Thread done - unload buffers and free memory
        while (m_queue->pop(iqBuffer))
        {
            if (m_debugMessages)
            {
                std::cout << "Free IQ buffer" << std::endl;
            }
            delete iqBuffer;
        }
        
        // Set the USRP device to minimum gain
        if (m_usrp != nullptr)
        {            
            if (m_debugMessages)
            {
                std::cout << "Set Tx gain to 0.0 dB" << std::endl;
            }
            m_usrp->set_tx_gain(0.0);
        }
    }
    
    void UHDTxSamples::quit()
    {
        m_sendThenQuit = false;
        m_done = true;
    }
    
    void UHDTxSamples::sendThenQuit()
    {
        m_sendThenQuit = true;
        m_done = true;
    }
}