#include "ButtonAdapter.h"

ButtonAdapter* ButtonAdapter::pButtonAdapters[MAX_BUTTONS];

bool ButtonAdapter::sFirst = true;

bool ButtonAdapter::begin() {
    ButtonAdapter::pButtonAdapters[mPin] = this;
    pinMode(mPin, INPUT);
    attachInterrupt(mPin, &ButtonAdapter::interrupt, CHANGE);

    mValue = digitalRead(mPin);
    
    return true;
}

bool ButtonAdapter::check() {
    return mChanged;
}

void ButtonAdapter::interrupt()
{
    for (int i=0;i < MAX_BUTTONS;i++)
    {
        if (ButtonAdapter::pButtonAdapters[i] != 0)
        {
            if (digitalRead(i) != ButtonAdapter::pButtonAdapters[i]->mValue)
            {
                ButtonAdapter::pButtonAdapters[i]->mValue = !ButtonAdapter::pButtonAdapters[i]->mValue;
                ButtonAdapter::pButtonAdapters[i]->mChanged = true;
            }
        }
    }
}


void ButtonAdapter::end()
{
    ButtonAdapter::pButtonAdapters[mPin] = 0;
    detachInterrupt(mPin);
}

bool ButtonAdapter::save(JsonObject* json) 
{
    if (mChanged)
    {
        const char* pMsg = mValue ? pMsgOn : pMsgOff;

        if (pMsg != NULL)
            (*json)[String("button") + mSuffix] = pMsg;
    }

    mChanged = false;
    return true;
}
