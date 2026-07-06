/*
	This is a SampVoice project file
	Author: CyberMor <cyber.mor.2020@gmail.ru>
	open.mp version author: AmyrAhmady (iAmir) <hhm6@yahoo.com>

	See more here https://github.com/AmyrAhmady/sampvoice

	Copyright (c) Daniel (CyberMor) 2020 All rights reserved
*/

#include "EffectManager.h"
#include <mutex>

void EffectManager::RegisterEffect(Effect* effect) {
    std::unique_lock lock(mutex_);
    effects_.insert(effect);
}

void EffectManager::UnregisterEffect(Effect* effect) {
    std::unique_lock lock(mutex_);
    effects_.erase(effect);
}

bool EffectManager::IsValidEffect(Effect* effect) {
    std::shared_lock lock(mutex_);
    return effects_.find(effect) != effects_.end();
}

std::set<Effect*> EffectManager::effects_;
std::shared_mutex EffectManager::mutex_;