#include "websocket_alg.h"
#include "variables/global.h"

void alg_controller::handleNewMessage(const WebSocketConnectionPtr& wsConnPtr, std::string &&message, const WebSocketMessageType &type)
{
    
}

void alg_controller::handleNewConnection(const HttpRequestPtr &req, const WebSocketConnectionPtr& wsConnPtr)
{
    connectedClients.insert(wsConnPtr);
    if (!isRunning){
        isRunning = true;
        alg_thread = boost::thread([this, wsConnPtr]() {
            while(isRunning){
                auto start = std::chrono::steady_clock::now();

                doJsonResponse();
                for(const auto& client : connectedClients){
                    client->send(json_resp.toStyledString());
                }
                json_resp.clear();

                auto waitTime = std::chrono::milliseconds(1000) - (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start));
                if (waitTime > std::chrono::milliseconds(0)){
                    std::this_thread::sleep_for(waitTime);
                }
            }
        });
    }
}

void alg_controller::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr)
{
    connectedClients.erase(wsConnPtr);

    if (connectedClients.size() == 0){
        isRunning = false;
        alg_thread.interrupt();
    }
}

void alg_controller::doJsonResponse()
{
    for (std::string path : paths){
        for (std::string field : fields){
            data = createJsonResponse(field, path);
            for (std::string element : data.getMemberNames()){
                Json::Value value = data[element];
                key = key_transform(element);

                if (findString(value, states[0]) && !value[states[0]].asBool()){
                    json_resp[key] = "active";
                } else if (findString(value, states[1]) && value[states[1]].asBool()){
                    json_resp[key] = "positive";
                } else if (findString(value, states[1]) && !value[states[1]].asBool() && field != "PROVODA"){
                    json_resp[key] = "intermediate";
                } else if (findString(value, states[2]) && !value[states[2]].asBool() && field != "PROVODA"){
                    json_resp[key] = "negative";
                } else {
                    json_resp[key] = "active";
                }
                if (value.isObject()){
                    if (key == "server") break;
                    for (std::string device_key : value.getMemberNames()){
                        if (device_key == devices[0] || device_key == devices[1]){
                            if (device_key == "K1"){
                                dev_key = "1k";
                            } else {
                                dev_key = "2k";
                            }

                            Json::Value second_value = value[device_key];
                            if (findString(second_value, states[0]) && !second_value[states[0]].asBool()){
                                json_resp[key + "_" + dev_key] = "active";
                            } else if (findString(second_value, states[1]) && second_value[states[1]].asBool()){
                                json_resp[key + "_" + dev_key] = "positive";
                            } else if (findString(second_value, states[2]) && !second_value[states[2]].asBool() && field != fields[5] && field != fields[6]){
                                json_resp[key + "_" + dev_key] = "negative";
                            } else {
                                json_resp[key + "_" + dev_key] = "positive";
                            }
                        }
                    }
                }
            }
        }
    }
    std::string sign = modeSignsManager::getModeName();
    if (sign == "absent"){
        mode = none;
    } else if (sign == "TEST"){
        mode == testing;
    } else if (sign == "WORK"){
        mode = working;
    } else {
        mode == rest;
    }

    for (std::string mode_elem : mode){
        auto element = key_transform(mode_elem);
        if(findString(json_resp, element)){
            json_resp[element] = "inactive";
        }
        else break;
    }
}

Json::Value alg_controller::createJsonResponse(const std::string& element, const std::string& path)
{
    auto clientRun = HttpRequest::newHttpRequest();
    clientRun->setMethod(drogon::Get);
    clientRun->setPath(path);
    auto [code, response] = variables.client->sendRequest(clientRun);
    Json::Value jsonResponse = *(response->getJsonObject());
    return jsonResponse.get(element, Json::Value());
}

std::string alg_controller::key_transform(const std::string& input)
{   
    std::string result = std::regex_replace(input, std::regex(R"([()])"), "_");
    result = std::regex_replace(result, std::regex("-"), "_");
    result = std::regex_replace(result, std::regex("К1|K1"), "1k");
    result = std::regex_replace(result, std::regex("К2|K2"), "2k");
    result = std::regex_replace(result, std::regex(" – |_+$"), "");
    return result;
}

bool alg_controller::findString(const Json::Value& json, const std::string& str)
{
    if (json.isArray()){
        for (const auto& item : json){
            if (item.isString() && item.asString() == str) {
                return true;
            }
        }
    } else if (json.isObject()){
        for (auto it = json.begin(); it != json.end(); ++it){
            if (it.key().isString() && it.key().asString() == str){
                return true;
            }
            if (it->isString() && it->asString() == str){
                return true;
            }
        }
    }
    return false;
}
