#pragma once
#include "../NKFM_UILayout.h"
#include "NKFM_UILayout_240x135.h"

static NKFM_UILayout_240x135 _nkfmSkin135Classic;

static NKFM_UILayout* const NKFM_SKINS[] = {
  &_nkfmSkin135Classic
};

constexpr uint8_t NKFM_SKIN_COUNT = sizeof(NKFM_SKINS) / sizeof(*NKFM_SKINS);
