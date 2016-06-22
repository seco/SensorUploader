#include "SensorChainedAdapterBase.h"
#include "Arduino.h"

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
};
bool SensorChainedAdapterBase::saveAll(JsonObject* obj) {
    bool res = true;

    for(SensorChainedAdapterBase* pThis = this; pThis != NULL; pThis = pThis->next()) {
        res &= pThis->save(obj);
    }

    return res;
}


bool SensorChainedAdapterBase::checkAll() {
    for(SensorChainedAdapterBase* pThis = this; pThis != NULL; pThis = pThis->next()) {
        if (pThis->check())
            return true;
    }

    return false;
}
