#pragma once
#include "../NKFM_UILayout.h"
#include "NKFM_UILayout_240x240.h"
#include "NKFM_UILayout_240x240_Arcada.h"
#include "NKFM_UILayout_240x240_KH.h"

static NKFM_UILayout_240x240        _nkfmSkin240Classic;
static NKFM_UILayout_240x240_Arcada _nkfmSkin240Arcada;
static NKFM_UILayout_240x240_KH     _nkfmSkin240KH;

static NKFM_UILayout* const NKFM_SKINS[] = {
  &_nkfmSkin240Classic,
  &_nkfmSkin240Arcada,
  &_nkfmSkin240KH
};

constexpr uint8_t NKFM_SKIN_COUNT = sizeof(NKFM_SKINS) / sizeof(*NKFM_SKINS);
