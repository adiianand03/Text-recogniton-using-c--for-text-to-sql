// UTIL_HPP

#ifndef UTIL_HPP
#define UTIL_HPP

#include <iostream>
#include <string>

using namespace std;

const char *API_TOKEN = "Authorization: Bearer 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";
const string MYSQL_HOST = "tcp://127.0.0.1:3306";
const string MYSQL_USER = "wsl_root";
const string MYSQL_PASSWD = "password";
const string MYSQL_TABLE = "adi";

const string SCHEMA = "| tally1 | purchase_orders : purchase_id, supplier_id, product_id, order_date, delivery_date, total_amount, status_purchase, payment_type, acc_no, quantity | sales_orders : sales_id, customer_id, product_id, order_date, delivery_date, total_amount, status_purchase, payment_type, acc_no, quantity | suppliers : supplier_id, supplier_name, addrss, phone_no | bank_details : acc_no, acc_name, balance, last_transaction | customer : customer_id, customer_name, phone_no, addrss | inventry: product_id, product_name, quantity, per_unit_price ";

#endif // UTIL_HPP
