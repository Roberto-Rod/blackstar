#include <csignal>
#include <unistd.h>
#include <uhd/utils/safe_main.hpp>
#include <boost/atomic.hpp>
#include <boost/thread/thread.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include "gpsgen.hpp"
#include "uhd_tx_samples.hpp"

bool debug { false };
boost::atomic<bool> done { false };

void sigHandler(int)
{
    std::cout << "QUITTING" << std::endl;
    done = true;
}

int UHD_SAFE_MAIN(int argc, char *argv[])
{
    std::signal(SIGINT, &sigHandler);

    boost::lockfree::spsc_queue<blackstar::iq_buff_t*, boost::lockfree::capacity<16>> queue;

    // Build Tx streamer
    blackstar::UHDTxSamples tx;
    tx.build(&queue, argc, argv);
    // Reset optarg index
    optind = 1;
    if (debug)
    {
        tx.enableDebugMessages();
    }

    // Build GPS generator
    blackstar::GPSGen gps;
    gps.build(&queue, argc, argv);
    if (debug)
    {
        gps.enableDebugMessages();
    }

    // Initialise SDR
    if (!tx.initialiseSDR())
    {
        std::cerr << "ERROR: Failed to initialise Tx streamer" << std::endl;
        return 1;
    }

    boost::thread consumer(boost::bind(&blackstar::UHDTxSamples::start, &tx));
    boost::thread producer(boost::bind(&blackstar::GPSGen::start, &gps));

    while (!done)
    {
        if (producer.timed_join(boost::posix_time::seconds(0)))
        {
            tx.sendThenQuit();
            consumer.join();
            done = true;
        }
    }

    gps.quit();
    tx.quit();

    producer.join();
    consumer.join();

    return 0;
}