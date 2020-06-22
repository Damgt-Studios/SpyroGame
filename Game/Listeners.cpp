#include "pchgame.h"
#include "Listeners.h"


void AudioSourceEvent::HandleEvent(ADEvents::ADEvent* _event)
{
    audioSource.Play();
}

std::string AudioSourceEvent::ToString()
{
    return this->audioSource.soundName;
}

void ParticleEmitterEvent::HandleEvent(ADEvents::ADEvent* _event)
{
    emitter.Activate(lifespan, *(XMFLOAT4*)_event->Parameter());
}

std::string ParticleEmitterEvent::ToString()
{
    return "Fountain Emitter";
}

void ApplyEffectEvent::SetTarget(ADResource::ADGameplay::GameObject* _target, ADResource::ADGameplay::Effect* _effect)
{
    target = _target;
    effect = _effect;
}

void ApplyEffectEvent::HandleEvent(ADEvents::ADEvent* _event)
{
    target->effects.push_back(effect->clone());
    target->effects[target->effects.size() - 1].get()->OnApply(target->GetStatSheet());
}

std::string ApplyEffectEvent::ToString()
{
    return effect->name;
}

void SetUIVisibilityListener::HandleEvent(ADEvents::ADEvent* _event)
{
    if (isKeyThreshold)
    {
        if (static_cast<int>(reinterpret_cast<intptr_t>(_event->Parameter())) >= keyIndex)
        {
            uiComponent->visible = isOnSwitch;
        }
        else if (switchFailures)
        {
            uiComponent->visible = !isOnSwitch;
        }
    }
    else
    {
        if (static_cast<int>(reinterpret_cast<intptr_t>(_event->Parameter())) == keyIndex)
        {
            uiComponent->visible = isOnSwitch;
        }
        else if(switchFailures)
        {
            uiComponent->visible = !isOnSwitch;
        }
    }
}

std::string SetUIVisibilityListener::ToString()
{
    std::string value = "UI Visibility Listener: ";
    value.append(uiComponent->name);
    value.append(". With key ");
    value.append(to_string(keyIndex));
    return value;
}

void SetUIFocusListener::HandleEvent(ADEvents::ADEvent* _event)
{
    if (isKeyThreshold)
    {
        if (static_cast<int>(reinterpret_cast<intptr_t>(_event->Parameter())) >= keyIndex)
        {
            uiComponent->Focus(isOnSwitch);
        }
        else if (switchFailures)
        {
            uiComponent->Focus(!isOnSwitch);
        }
    }
    else
    {
        if (static_cast<int>(reinterpret_cast<intptr_t>(_event->Parameter())) == keyIndex)
        {
            uiComponent->Focus(isOnSwitch);
        }
        else if (switchFailures)
        {
            uiComponent->Focus(!isOnSwitch);
        }
    }

}

std::string SetUIFocusListener::ToString()
{
    return "";
}

void SetUISetFrameListener::HandleEvent(ADEvents::ADEvent* _event)
{
    //I probably should put a check here...
    image->SetCurrentFrame(static_cast<int>(reinterpret_cast<intptr_t>(_event->Parameter())));
}

std::string SetUISetFrameListener::ToString()
{
    return "";
}

void SetUITilingListener::HandleEvent(ADEvents::ADEvent* _event)
{
    int value = static_cast<int>(reinterpret_cast<intptr_t>(_event->Parameter()));
    if (value < 0)
        value = 0;
    image->tiled = value;
}

std::string SetUITilingListener::ToString()
{
    return "";
}

void SetUISelectionListener::HandleEvent(ADEvents::ADEvent* _event)
{
    int value = static_cast<int>(reinterpret_cast<intptr_t>(_event->Parameter()));
    if (value < 0)
        value = 0;
    list->SetSelected(value);
}

std::string SetUISelectionListener::ToString()
{
    return std::string();
}

void SetUITextListener::HandleEvent(ADEvents::ADEvent* _event)
{
    label->output = *static_cast<std::string*>(_event->Parameter());
}

std::string SetUITextListener::ToString()
{
    return std::string();
}
