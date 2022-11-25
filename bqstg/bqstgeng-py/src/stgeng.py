#!/usr/bin/python3

# -*- coding: utf-8 -*-
import sys
import getopt
import datetime
import json

sys.path.append(".")

from bqstgeng import *


class StgInstTaskHandlerBase(object):
    def __init__(self, e):
        self.stg_eng = e

    def on_stg_start(self):
        pass

    def on_stg_inst_start(self, stg_inst_info):
        pass

    def on_stg_inst_timer(self, stg_inst_info):
        pass

    def on_push_topic(self, stg_inst_info, topic_content):
        pass

    def on_order_ret(self, stg_inst_info, order_info):
        pass

    def on_cancel_order_ret(self, stg_inst_info, order_info):
        pass

    def on_trades(self, stg_inst_info, trades):
        market_data = json.dumps(trades)

    def on_books(self, stg_inst_info, books):
        market_data = json.dumps(books)

    def on_tickers(self, stg_inst_info, tickers):
        market_data = json.dumps(tickers)

    def on_candle(self, stg_inst_info, candle):
        market_data = json.dumps(candle)

    def on_stg_inst_add(self, stg_inst_info):
        pass

    def on_stg_inst_del(self, stg_inst_info):
        pass

    def on_stg_inst_chg(self, stg_inst_info):
        pass

    def on_pos_update_of_acct_id(self, stg_inst_info, pos_snapshot):
        pass

    def on_pos_snapshot_of_acct_id(self, stg_inst_info, pos_snapshot):
        pass

    def on_pos_update_of_stg_id(self, stg_inst_info, pos_snapshot):
        pass

    def on_pos_snapshot_of_stg_id(self, stg_inst_info, pos_snapshot):
        pass

    def on_pos_update_of_stg_inst_id(self, stg_inst_info, pos_snapshot):
        pass

    def on_pos_snapshot_of_stg_inst_id(self, stg_inst_info, pos_snapshot):
        pass

    def on_assets_update(self, stg_inst_info, assets_update):
        pass

    def on_assets_snapshot(self, stg_inst_info, assets_snapshot):
        pass
