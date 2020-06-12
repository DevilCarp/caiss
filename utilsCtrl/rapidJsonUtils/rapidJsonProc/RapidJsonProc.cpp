//
// Created by Chunel on 2020/5/24.
//

#include "RapidJsonProc.h"
#include <string>


inline static std::string buildDistanceType(CAISS_DISTANCE_TYPE type) {
    std::string ret;
    switch (type) {
        case CAISS_DISTANCE_EUC:
            ret = "euclidean";
            break;
        case CAISS_DISTANCE_INNER:
            ret = "cosine";
            break;
        case CAISS_DISTANCE_EDITION:
            ret = "edition";
            break;
        default:
            break;
    }

    return ret;
}

RapidJsonProc::RapidJsonProc() {
}

RapidJsonProc::~RapidJsonProc() {
}


CAISS_RET_TYPE RapidJsonProc::init() {
    CAISS_FUNCTION_END
}

CAISS_RET_TYPE RapidJsonProc::deinit() {
    CAISS_FUNCTION_END
}


CAISS_RET_TYPE RapidJsonProc::parseInputData(const char *line, AnnDataNode& dataNode) {
    CAISS_ASSERT_NOT_NULL(line)

    CAISS_FUNCTION_BEGIN

    Document dom;
    dom.Parse(line);    // data是一行数据，形如：{"hello" : [1,0,0,0]}

    if (dom.HasParseError()) {
        return CAISS_RET_JSON;
    }

    Value& jsonObject = dom;
    if (!jsonObject.IsObject()) {
        return CAISS_RET_JSON;
    }

    for (Value::ConstMemberIterator itr = jsonObject.MemberBegin(); itr != jsonObject.MemberEnd(); ++itr) {
        dataNode.index = itr->name.GetString();    // 获取行名称
        rapidjson::Value& array = jsonObject[dataNode.index.c_str()];
        for (unsigned int i = 0; i < array.Size(); ++i) {
            dataNode.node.push_back((CAISS_FLOAT)strtod(array[i].GetString(), nullptr));
        }
    }

    CAISS_FUNCTION_END
}


CAISS_RET_TYPE RapidJsonProc::buildSearchResult(const std::list<AnnResultDetail> &details,
                                              CAISS_DISTANCE_TYPE distanceType, std::string &result) {
    CAISS_FUNCTION_BEGIN

    Document dom;
    dom.SetObject();

    Document::AllocatorType& alloc = dom.GetAllocator();
    dom.AddMember("version", CAISS_VERSION, alloc);
    dom.AddMember("size", details.size(), alloc);

    std::string type = buildDistanceType(distanceType);    // 需要在这里开一个string，然后再构建json。否则release版本无法使用
    dom.AddMember("distance_type", StringRef(type.c_str()), alloc);

    rapidjson::Value array(rapidjson::kArrayType);

    for (const AnnResultDetail& detail : details) {
        rapidjson::Value obj(rapidjson::kObjectType);

        obj.AddMember("distance", (detail.distance < 0.00001) ? (0.0f) : detail.distance, alloc);
        obj.AddMember("index", detail.index, alloc);    // 这里的index，表示的是这属于模型中的第几个节点(注：跟算法类中，index和label的取名正好相反)
        obj.AddMember("label", StringRef(detail.label.c_str()), alloc);
//        rapidjson::Value node(rapidjson::kArrayType);    // 输出向量的具体内容，暂时不需要了
//        for (auto j : detail.node) {
//            node.PushBack(j, alloc);
//        }
//        obj.AddMember("node", node, alloc);
        array.PushBack(obj, alloc);
    }

    dom.AddMember("details", array, alloc);

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    dom.Accept(writer);
    result = buffer.GetString();    // 将最终的结果值，赋值给result信息，并返回

    CAISS_FUNCTION_END
}


