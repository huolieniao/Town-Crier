
#include <Log.h>
#include <string>
#include <Debug.h>

#include "stdint.h"
#include "event_handler.h"
#include "scrapers.h"
#include "scrapers/flight.cpp"
#include "scrapers/utils.h"
#include "time.h"
#include "eth_transaction.h"
#include "eth_abi.h"
#include "Enclave_t.h"
#include "external/keccak.h"
#include "Constants.h"

static int stock_ticker_handler(int nonce, uint64_t request_id, uint8_t request_type,
                                const uint8_t *req, int req_len, uint8_t *raw_tx, size_t *raw_tx_len)
{
    /*
    int ret;
    if (req_len != 64)
    {
        LL_CRITICAL("req_len is not 64");
        return -1;
    }
    
    char* code = (char*)(req);
    uint32_t date;
    time_t epoch;
    memcpy(&date, req + 64 - sizeof (uint32_t), sizeof (uint32_t));
    date = swap_uint32(date);

    epoch = date;
    LL_NOTICE("Looking for %s at %lld", code, epoch);

    int price = (int) get_closing_price(12, 3, 2014, "BABA");

    LL_NOTICE("Closing pricing is %d", price);

    bytes rr;
    enc_int(rr, price, sizeof (price));;

    ret = get_raw_signed_tx(nonce, 32, 
        request_id, request_type, 
        req, req_len, 
        rr,
        raw_tx, raw_tx_len);

    return ret;
    */
    return 0; 
}



int handle_request(int nonce,
                   uint64_t id,
                   uint64_t type,
                   uint8_t *data,
                   int data_len,
                   uint8_t *raw_tx,
                   size_t *raw_tx_len)
{
    int ret;
    bytes resp_data;
    int resp_data_len = 0;
/*
    printf_sgx("nonce: %d\n", nonce);
    printf_sgx("id: %llu\n", id);
    printf_sgx("type: %llu\n", type);
    printf_sgx("data len: %d\n", data_len);
    for (int i = 0; i < data_len; i++)
        printf_sgx("%u,", data[i]);
    printf_sgx("\n");
*/
    switch (type)
    {
    case TYPE_FINANCE_INFO:
        return stock_ticker_handler(nonce, id, type,
            data, data_len,
            raw_tx, raw_tx_len);
        break;
    case TYPE_FLIGHT_INS:
        {
            FlightScraper flightHandler;

            int found;
            switch (flightHandler.handler(data,data_len, &found))
            {
                case INVALID_PARAMS:
                    return -1; //TODO
                case WEB_ERROR:
                    return -1; //TODO
                case NO_ERROR:
                    enc_int(resp_data, found, sizeof(found));
                    break;
            };

        }
    case TYPE_STEAM_EX:
        {
            //TODO
//             int found = 0;
//             if (data_len != 6 * 32)
//             {
//                 LL_CRITICAL("data_len %d*32 is not 6*32", data_len / 32);
//                 return -1;
//             }
//             ret = handler_steam_exchange(data, data_len, &found);
//             if (ret == -1)
//             {
//                 LL_CRITICAL("%s returns %d", "handler_steam_exchange", ret);
//                 return -1;
//             }
// //            found = 1;
//             enc_int(resp_data, found, sizeof ( found ));
//             resp_data_len = 32;
            break;
        }
    case TYPE_CURRENT_VOTE:
        {
            double r1 = 0, r2 = 0, r3 = 0;
            long long time1, time2;

            rdtsc(&time1);
            yahoo_current("GOOG", &r1);
            rdtsc(&time2);
            LL_CRITICAL("Yahoo: %llu", time2 - time1);

            google_current("GOOG", &r3);
            rdtsc(&time1);
            LL_CRITICAL("Bloomberg: %llu", time1 - time2);

            google_current("GOOG", &r2);
            rdtsc(&time2);
            LL_CRITICAL("GOOGLE: %llu", time2 - time1);

            break;
        }
    default :
        LL_CRITICAL("Unknown request type: %d", type);
        return -1;
        break;
    }

    //sign transactions
    ret = get_raw_signed_tx(nonce, 32, id, type, data, data_len, resp_data, raw_tx, raw_tx_len);
    /*
    printf_sgx("raw tx len: %d\n", *raw_tx_len);
    for (int i = 0; i < *raw_tx_len; i++)
        printf_sgx("%u,", raw_tx[i]);
    printf_sgx("\n");
     */
    return ret;
}

