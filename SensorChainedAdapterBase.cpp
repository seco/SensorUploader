#include "SensorChainedAdapterBase.h"


bool SensorChainedAdapterBase::beginAll() {
    for(SensorChainedAdapterBase* pThis = this; pThis != NULL; pThis = pThis->next()) {
        if (!pThis->begin()) {
            for(SensorChainedAdapterBase* pThis1 = this; pThis != pThis1; pThis1 = pThis1->next()) {
                pThis1->end();
            }
            return false;
        }
    }

    return true;
}

void SensorChainedAdapterBase::endAll() {
    for(SensorChainedAdapterBase* pThis = this; pThis != NULL; pThis = pThis->next()) {
        pThis->end();
    }
}

void SensorChainedAdapterBase::applyAll(SENSOR_CALLBACK_SIGNATURE) {
    for(SensorChainedAdapterBase* pThis = this; pThis != NULL; pThis = pThis->next()) {
		callback(pThis);
    }
}



bool SensorChainedAdapterBase::saveAll(JsonObject* obj) {
    bool res = true;

    for(SensorChainedAdapterBase* pThis = this; pThis != NULL; pThis = pThis->next()) {
        res &= pThis->save(obj);
    }

    return res;
}

bool SensorChainedAdapterBase::processAll(char* topic, char* payload) {
    for(SensorChainedAdapterBase* pThis = this; pThis != NULL; pThis = pThis->next()) {
		const char* pTopic = pThis->topic();

        if (pTopic && topic && topic[0] && pTopic[0] && !strcmp(pTopic, topic) && pThis->process(topic, payload))
            return true;
    }

    return false;
}


bool SensorChainedAdapterBase::checkAll() {
    for(SensorChainedAdapterBase* pThis = this; pThis != NULL; pThis = pThis->next()) {
        if (pThis->check())
            return true;
    }

    return false;
}
