#pragma once

#include <drogon/WebSocketController.h>
#include <drogon/PubSubService.h>
#include <drogon/HttpAppFramework.h>
#include <drogon/drogon.h>

#include <iostream>
#include <string>
#include <unordered_map>
#include <chrono>
#include <boost/thread.hpp>
#include <atomic>
#include <regex>

using namespace drogon;

class alg_controller : public drogon::WebSocketController<alg_controller>
{
  public:
    boost::thread alg_thread;
    Json::Value json_resp;

    std::vector<std::string> paths = {"/status", "/status_com"};
    std::vector<std::string> fields = {"server", "server2", "server3", "server4", "server5", "commutators", "PROVODA"};
    std::vector<std::string> devices = {"K1", "K2"};
    std::vector<std::string> states = {"availability", "serviceability", "performance"};
    std::string key;
    std::string dev_key;
    Json::Value data;
    std::set<WebSocketConnectionPtr> connectedClients;
    bool isRunning = false;

    std::vector<std::string> mode;
    std::vector<std::string> none = {"station(1)", "station(2)", "station(3)", "station(4)", "station(5)", "station(6)", "server", "commutator(1)", "commutator(2)", "commutator(3)", "commutator(4)", "station(1) – commutator(1)", "station(1) – commutator(2)", "station(1) – commutator(3)", "station(1) – commutator(4)", "station(1) – commutator(5)", "station(1) – commutator(6)", "station(1) – server(К1)", "station(2) – commutator(1)", "station(2) – commutator(2)", "station(2) – commutator(3)", "station(2) – commutator(4)", "station(2) – commutator(5)", "station(2) – commutator(6)", "station(2) – server(К2)"};
    std::vector<std::string> testing;
    std::vector<std::string> working = {"server", "station(1)", "station(2)", "station(3)", "station(4)", "station(1) – server(К1)", "station(2) – server(К2)"};
    std::vector<std::string> rest = {"commutator(2)", "station(1)", "station(2)", "station(3)", "station(4)"};
    
    virtual void handleNewMessage(const WebSocketConnectionPtr& wsConnPtr, std::string&& message, const WebSocketMessageType& type) override;
    virtual void handleNewConnection(const HttpRequestPtr& req, const WebSocketConnectionPtr& wsConnPtr) override;
    virtual void handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr) override;
    Json::Value createJsonResponse(const std::string& element, const std::string& path);
    std::string key_transform(const std::string& input);
    bool findString(const Json::Value& j, const std::string& str);
    void doJsonResponse();

    WS_PATH_LIST_BEGIN
    WS_PATH_ADD("/status_system");
    WS_PATH_LIST_END
};
