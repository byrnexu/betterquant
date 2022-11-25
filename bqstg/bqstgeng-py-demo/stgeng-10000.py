#!/usr/bin/python3

# -*- coding: utf-8 -*-
import sys
import getopt
import datetime
import time
import json
from stgeng import *

sys.path.append(".")


class StgInstTaskHandler(StgInstTaskHandlerBase):
    def on_stg_start(self):
        # Install a timer for stg inst 1 that fires 1 time after 3000 ms
        self.stg_eng.install_stg_inst_timer(
            1, "testTimer", ExecAtStartup.IsFalse, 3000, 1
        )

        now = int(time.time() * 1000000)

        ret_of_qry = self.stg_eng.query_his_md_between_2_ts(
            "MD@Binance@Spot@BTC-USDT@Trades", now - 60 * 1000000, now, 1
        )

        ret_of_qry = self.stg_eng.query_his_md_between_2_ts(
            MarketCode.Binance,
            SymbolType.Spot,
            "BTC-USDT",
            MDType.Trades,
            now - 60 * 1000000,
            now,
            1,
        )

        ret_of_qry = self.stg_eng.query_specific_num_of_his_md_before_ts(
            "MD@Binance@Spot@BTC-USDT@Trades", now, 1, 1
        )

        ret_of_qry = self.stg_eng.query_specific_num_of_his_md_before_ts(
            MarketCode.Binance, SymbolType.Spot, "BTC-USDT", MDType.Trades, now, 1, 1
        )

        ret_of_qry = self.stg_eng.query_specific_num_of_his_md_after_ts(
            "MD@Binance@Spot@BTC-USDT@Trades", now - 60 * 1000000, 2, 1
        )

        ret_of_qry = self.stg_eng.query_specific_num_of_his_md_after_ts(
            MarketCode.Binance,
            SymbolType.Spot,
            "BTC-USDT",
            MDType.Trades,
            now - 60 * 1000000,
            2,
            1,
        )

        ret_of_qry = self.stg_eng.query_specific_num_of_his_md_before_ts(
            "MD@Binance@Spot@BTC-USDT@Books", now - 60 * 1000000, 2, 20
        )

        print(ret_of_qry[0])
        print(ret_of_qry[1])

    def on_stg_inst_start(self, stg_inst_info):
        if stg_inst_info.stg_inst_id == 1:
            # sub topic
            self.stg_eng.sub(
                stg_inst_info.stg_inst_id, "shm://MD.Binance.Spot/NewSymbol"
            )

            # sub market data of trades, note that the topic is case sensitive.
            self.stg_eng.sub(
                stg_inst_info.stg_inst_id, "shm://MD.Binance.Spot/ADA-USDT/Trades"
            )
            # sub market data of tickers
            self.stg_eng.sub(
                stg_inst_info.stg_inst_id, "shm://MD.Binance.Spot/ADA-USDT/Tickers"
            )
            # sub market data of books
            self.stg_eng.sub(
                stg_inst_info.stg_inst_id, "shm://MD.Binance.Spot/BTC-USDT/Books/1"
            )
            # sub market data of candle
            self.stg_eng.sub(
                stg_inst_info.stg_inst_id, "shm://MD.Binance.Spot/ADA-USDT/Candle"
            )

            # sub trades of BNB-USDT for calc pnl, note that the topic is case sensitive.
            self.stg_eng.sub(
                stg_inst_info.stg_inst_id, "shm://MD.Binance.Spot/BNB-USDT/Trades"
            )

            # sub assets info of acct 10001, note that the topic is case sensitive.
            self.stg_eng.sub(
                stg_inst_info.stg_inst_id,
                "shm://RISK.PubChannel.Trade/AssetInfo/AcctId/10001",
            )
            # sub pos info of acct 10003, note that the topic is case sensitive.
            self.stg_eng.sub(
                stg_inst_info.stg_inst_id,
                "shm://RISK.PubChannel.Trade/PosInfo/AcctId/10003",
            )
            # sub pos info of stg id 10000, note that the topic is case sensitive.
            self.stg_eng.sub(
                stg_inst_info.stg_inst_id,
                "shm://RISK.PubChannel.Trade/PosInfo/StgId/10000",
            )
            # sub pos info of stg inst id 2, note that the topic is case sensitive.
            self.stg_eng.sub(
                stg_inst_info.stg_inst_id,
                "shm://RISK.PubChannel.Trade/PosInfo/StgId/10000/StgInstId/2",
            )

        if stg_inst_info.stg_inst_id == 2:
            pass

    def on_stg_inst_timer(self, stg_inst_info):
        ret_of_order = self.stg_eng.order(
            stg_inst_info,
            acct_id=10001,
            symbol_code="ADA-USDT",
            side=Side.Ask,
            pos_side=PosSide.Both,
            order_price=0.300,
            order_size=35,
        )

        status_code = ret_of_order[0]
        if status_code != 0:
            status_msg = get_status_msg(status_code)
            print(f"Create order failed. {status_code} - {status_msg}")
            return

        order_id = ret_of_order[1]
        print(f"Create order info of order_id {order_id} success. ")

        ret_of_get_order_info = self.stg_eng.get_order_info(order_id)
        status_code = ret_of_get_order_info[0]
        if status_code != 0:
            status_msg = get_status_msg(status_code)
            print(f"Get order info failed. {status_code} - {status_msg}")
            return

        order_info = ret_of_get_order_info[1]
        print(f"Get order info of {order_id} success. {order_info.to_short_str()}")
        print(f'order_info.symbol_code = {"".join(order_info.symbol_code)}')
        print(f'order_info.exch_symbol_code = {"".join(order_info.exch_symbol_code)}')

    def on_push_topic(self, stg_inst_info, topic_data):
        print(topic_data)

    def on_order_ret(self, stg_inst_info, order_info):
        print(f"on order ret {order_info.to_short_str()}")

    def on_cancel_order_ret(self, stg_inst_info, order_info):
        print(f"on cancel order ret {order_info.to_short_str()}")

    def on_trades(self, stg_inst_info, trades):
        market_data = json.dumps(trades)

    def on_books(self, stg_inst_info, books):
        market_data = json.dumps(books)

    def on_tickers(self, stg_inst_info, tickers):
        market_data = json.dumps(tickers)

    def on_candle(self, stg_inst_info, candle):
        market_data = json.dumps(candle)

    def on_stg_inst_add(self, stg_inst_info):
        # Write strategy business code here
        pass

    def on_stg_inst_del(self, stg_inst_info):
        # Write strategy business code here
        pass

    def on_stg_inst_chg(self, stg_inst_info):
        # Write strategy business code here
        pass

    def on_pos_update_of_acct_id(self, stg_inst_info, pos_snapshot):
        # query pnl of acctId=10001
        if stg_inst_info.stg_inst_id == 1:
            ret_of_query_pnl = pos_snapshot.query_pnl(
                query_cond="acctId=10001", quote_currency_for_calc="USDT"
            )
            status_code = ret_of_query_pnl[0]
            if status_code == 0:
                pnl = ret_of_query_pnl[1]
                print(f"pnl update = {pnl.to_str()}")
            else:
                status_msg = get_status_msg(status_code)
                print(f"{status_code} - {status_msg}")

    def on_pos_snapshot_of_acct_id(self, stg_inst_info, pos_snapshot):
        # query pnl of acctId=10001
        if stg_inst_info.stg_inst_id == 1:
            ret_of_query_pnl = pos_snapshot.query_pnl(
                query_cond="acctId=10001", quote_currency_for_calc="USDT"
            )
            status_code = ret_of_query_pnl[0]
            if status_code == 0:
                pnl = ret_of_query_pnl[1]
                print(f"pnl snapashot = {pnl.to_str()}")
            else:
                status_msg = get_status_msg(status_code)
                print(f"{status_code} - {status_msg}")

    def on_pos_update_of_stg_id(self, stg_inst_info, pos_snapshot):
        # query pnl of stgId=10000
        if stg_inst_info.stg_inst_id == 1:
            ret_of_query_pnl = pos_snapshot.query_pnl(
                query_cond="stgId=10000", quote_currency_for_calc="USDT"
            )
            status_code = ret_of_query_pnl[0]
            if status_code == 0:
                pnl = ret_of_query_pnl[1]
                print(f"pnl = {pnl.to_str()}")
                self.stg_eng.save_to_db(pnl)
            else:
                status_msg = get_status_msg(status_code)
                print(f"{status_code} - {status_msg}")

        # query pnl of stgId=10000&stgInstId=2
        if stg_inst_info.stg_inst_id == 1:
            ret_of_query_pnl = pos_snapshot.query_pnl(
                query_cond="stgId=10000&stgInstId=2", quote_currency_for_calc="USDT"
            )
            status_code = ret_of_query_pnl[0]
            if status_code == 0:
                pnl = ret_of_query_pnl[1]
                print(f"pnl = {pnl.to_str()}")
            else:
                status_msg = get_status_msg(status_code)
                print(f"{status_code} - {status_msg}")
        # query pnl of stgId=10000&stgInstId=1
        if stg_inst_info.stg_inst_id == 1:
            ret_of_query_pnl = pos_snapshot.query_pnl(
                query_cond="stgId=10000&stgInstId=1", quote_currency_for_calc="USDT"
            )
            status_code = ret_of_query_pnl[0]
            if status_code == 0:
                pnl = ret_of_query_pnl[1]
                print(f"pnl = {pnl.to_str()}")
            else:
                status_msg = get_status_msg(status_code)
                print(f"{status_code} - {status_msg}")

        # get pos info detail
        if stg_inst_info.stg_inst_id == 1:
            pos_info_detail = pos_snapshot.get_pos_info_detail()
            for rec in pos_info_detail:
                pos_info_key = rec.key()
                pos_info = rec.data()
                print(f"pos_info_key = {pos_info_key}")
                print(f"pos_info = {pos_info.to_str()}")

        # query pos info group by stgId and stgInstId, note that the group
        # condition is case sensitive.
        if stg_inst_info.stg_inst_id == 1:
            ret_of_query_pos_info_group = pos_snapshot.query_pos_info_group_by(
                "stgId&stgInstId"
            )
            status_code = ret_of_query_pos_info_group[0]
            if status_code == 0:
                rec_set = ret_of_query_pos_info_group[1]
                for rec in rec_set:
                    pos_info_key = rec.key()
                    pos_info_group = rec.data()
                    print(f"pos_info_key = {pos_info_key}")
                    for pos_info in pos_info_group:
                        print(f"pos_info = {pos_info.to_str()}")
            else:
                status_msg = get_status_msg(status_code)
                print(f"{status_code} - {status_msg}")

        # query pos info group of stgId=10000 and stgInstId=1, note that the
        # query condition is case sensitive.
        if stg_inst_info.stg_inst_id == 1:
            ret_of_query_pos_info_group = pos_snapshot.query_pos_info_group(
                "stgId=10000&stgInstId=1"
            )
            status_code = ret_of_query_pos_info_group[0]
            if status_code == 0:
                pos_info_group = ret_of_query_pos_info_group[1]
                for pos_info in pos_info_group:
                    print(pos_info.to_str())
            else:
                status_msg = get_status_msg(status_code)
                print(f"{status_code} - {status_msg}")

        # query pnl group by stgId and stgInstId, note that the group condition
        # is case sensitive.
        if stg_inst_info.stg_inst_id == 1:
            ret_of_query_pnl_group = pos_snapshot.query_pnl_group_by(
                "stgId&stgInstId", "USDT"
            )
            status_code = ret_of_query_pnl_group[0]
            if status_code == 0:
                rec_set = ret_of_query_pnl_group[1]
                for rec in rec_set:
                    pnl_key = rec.key()
                    pnl = rec.data()
                    print(f"pnl_key = {pnl_key}")
                    print(f"pnl = {pnl.to_str()}")
            else:
                status_msg = get_status_msg(status_code)
                print(f"{status_code} - {status_msg}")

    def on_pos_snapshot_of_stg_id(self, stg_inst_info, pos_snapshot):
        pass

    def on_pos_update_of_stg_inst_id(self, stg_inst_info, pos_snapshot):
        # Write strategy business code here
        pass

    def on_pos_snapshot_of_stg_inst_id(self, stg_inst_info, pos_snapshot):
        # Write strategy business code here
        pass

    def on_assets_update(self, stg_inst_info, assets_update):
        for rec in assets_update:
            asset_info = rec.data()
            print(f"on assets update {asset_info.to_str()}")

    def on_assets_snapshot(self, stg_inst_info, assets_snapshot):
        for rec in assets_snapshot:
            asset_info = rec.data()
            print(f"on assets snapshot {asset_info.to_str()}")


def parse_argv(argv):
    conf = ""
    try:
        opts, args = getopt.getopt(argv[1:], "hc:", ["conf="])
    except getopt.GetoptError:
        print(f"{argv[0]} -c <configfile> or {argv[0]} --conf=<configfile>")
        sys.exit(2)
    for opt, arg in opts:
        if opt == "-h":
            print(f"{argv[0]} -c <configfile> or {argv[0]} --conf=<configfile>")
            sys.exit()
        elif opt in ("-c", "--conf"):
            conf = arg
    return conf


def main(argv):
    conf = parse_argv(argv)
    if len(conf) == 0:
        print(f"{argv[0]} -c <configfile> or {argv[0]} --conf=<configfile>")
        sys.exit(3)

    stg_eng = StgEng(conf)
    stg_inst_task_handler = StgInstTaskHandler(stg_eng)
    ret = stg_eng.init(stg_inst_task_handler)
    if ret != 0:
        sys.exit(4)
    stg_eng.run()


if __name__ == "__main__":
    main(sys.argv)
