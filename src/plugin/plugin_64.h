#pragma once
#include "plugin64.h"

typedef UINT64 DllErrorCode;

inline std::string BoolToString(bool b)
{
	return b ? "NG" : "OK";
}

#define PL( f ) BoolToString(f) + ":" +  #f + "\n"
#define P( f ) #f ":" + BoolToString(f)

struct DllError{
	union {
		DllErrorCode code;
		struct {
			bool unmatchdDateProc1Injector : 1;
		};

		std::string print() {
			return PL(unmatchdDateProc1Injector);
		}
	} date;

	union {
		DllErrorCode code;
		struct {
			bool unmatchdEventDialog1Injector : 1;
			bool unmatchdEventDialog2Injector : 1;
			bool unmatchdEventDialog3Injector : 1;
		};

		std::string print() {
			return PL(unmatchdEventDialog1Injector)
				+ PL(unmatchdEventDialog2Injector)
				+ PL(unmatchdEventDialog3Injector);
		}
	} eventDialog;

	union {
		DllErrorCode code;
		struct {
			bool unmatchdFileSaveProc1Injector : 1;
			bool unmatchdFileSaveProc2Injector : 1;
			bool unmatchdFileSaveProc3Injector : 1;
			bool unmatchdFileSaveProc4Injector : 1;
			bool unmatchdFileSaveProc5Injector : 1;
			bool unmatchdFileSaveProc6Injector : 1;
			bool unmatchdFileSaveProc7Injector : 1;
			bool unmatchdFileSaveProc8Injector : 1;
			bool unmatchdFileSaveProc9Injector : 1;
			bool unmatchdFileSaveProc10Injector : 1;
		};

		std::string print() {
			return PL(unmatchdFileSaveProc1Injector)
				+ PL(unmatchdFileSaveProc2Injector)
				+ PL(unmatchdFileSaveProc3Injector)
				+ PL(unmatchdFileSaveProc4Injector)
				+ PL(unmatchdFileSaveProc5Injector)
				+ PL(unmatchdFileSaveProc6Injector)
				+ PL(unmatchdFileSaveProc7Injector)
				+ PL(unmatchdFileSaveProc8Injector)
				+ PL(unmatchdFileSaveProc9Injector)
				+ PL(unmatchdFileSaveProc10Injector);
		}
	} fileSave;

	union {
		DllErrorCode code;
		struct {
			bool unmatchdCharCodePointLimiterPatchInjector : 1;
			bool unmatchdFontBufferHeapZeroClearInjector : 1;
			bool unmatchdFontBufferClear1Injector : 1;
			bool unmatchdFontBufferClear2Injector : 1;
			bool unmatchdFontBufferExpansionInjector : 1;
			bool unmatchdFontSizeLimitInjector : 1;
		};

		std::string print() {
			return PL(unmatchdCharCodePointLimiterPatchInjector)
				+ PL(unmatchdFontBufferHeapZeroClearInjector)
				+ PL(unmatchdFontBufferClear1Injector)
				+ PL(unmatchdFontBufferClear2Injector)
				+ PL(unmatchdFontBufferExpansionInjector)
				+ PL(unmatchdFontSizeLimitInjector);
		}
	} font;

	union {
		DllErrorCode code;
		struct {
			bool unmatchdImeProc1Injector : 1;
			bool unmatchdImeProc2Injector : 1;
			bool unmatchdImeProc3Injector : 1;
		};

		std::string print() {
			return PL(unmatchdImeProc1Injector)
				+ PL(unmatchdImeProc2Injector)
				+ PL(unmatchdImeProc3Injector);
		}
	} ime;

	union {
		DllErrorCode code;
		struct {
			bool unmatchdInputProc1Injector : 1;
			bool unmatchdInputProc2Injector : 1;
		};

		std::string print() {
			return PL(unmatchdInputProc1Injector)
				+ PL(unmatchdInputProc2Injector);
		}

	} input;

	union {
		DllErrorCode code;
		struct {
			bool unmatchdListFieldAdjustmentProc1Injector : 1;
			bool unmatchdListFieldAdjustmentProc2Injector : 1;
			bool unmatchdListFieldAdjustmentProc3Injector : 1;
			bool unmatchdListFieldAdjustmentProc4Injector : 1;
		};

		std::string print() {
			return PL(unmatchdListFieldAdjustmentProc1Injector)
				+ PL(unmatchdListFieldAdjustmentProc2Injector)
				+ PL(unmatchdListFieldAdjustmentProc3Injector)
				+ PL(unmatchdListFieldAdjustmentProc4Injector);
		}

	} listFiledAdjustment;

	union {
		DllErrorCode code;
		struct {
			bool unmatchdLocalizationProc1Injector : 1;
			bool unmatchdLocalizationProc2Injector : 1;
			bool unmatchdLocalizationProc3Injector : 1;
			bool unmatchdLocalizationProc4Injector : 1;
			bool unmatchdLocalizationProc5Injector : 1;
			bool unmatchdLocalizationProc6Injector : 1;
			bool unmatchdLocalizationProc7Injector : 1;
			bool unmatchdLocalizationProc8Injector : 1;
			bool unmatchdLocalizationProc9Injector : 1;
			bool unmatchdLocalizationProc10Injector : 1;
		};

		std::string print() {
			return PL(unmatchdLocalizationProc1Injector)
				+ PL(unmatchdLocalizationProc2Injector)
				+ PL(unmatchdLocalizationProc3Injector)
				+ PL(unmatchdLocalizationProc4Injector)
				+ PL(unmatchdLocalizationProc5Injector)
				+ PL(unmatchdLocalizationProc6Injector)
				+ PL(unmatchdLocalizationProc7Injector)
				+ PL(unmatchdLocalizationProc8Injector)
				+ PL(unmatchdLocalizationProc9Injector)
				+ PL(unmatchdLocalizationProc10Injector);
		}

	} localization;

	union {
		DllErrorCode code;
		struct {
			bool unmatchdMainTextProc1Injector : 1;
			bool unmatchdMainTextProc2Injector : 1;
			bool unmatchdMainTextProc3Injector : 1;
			bool unmatchdMainTextProc4Injector : 1;
		};

		std::string print() {
			return PL(unmatchdMainTextProc1Injector)
				+ PL(unmatchdMainTextProc2Injector)
				+ PL(unmatchdMainTextProc3Injector)
				+ PL(unmatchdMainTextProc4Injector);
		}
	} mainText;

	union {
		DllErrorCode code;
		struct {
			bool unmatchdMapAdjustmentProc1Injector : 1;
			bool unmatchdMapAdjustmentProc2Injector : 1;
			bool unmatchdMapAdjustmentProc3Injector : 1;
			bool unmatchdMapAdjustmentProc4Injector : 1;
			bool unmatchdMapAdjustmentProc5Injector : 1;
		};

		std::string print() {
			return PL(unmatchdMapAdjustmentProc1Injector)
				+ PL(unmatchdMapAdjustmentProc2Injector)
				+ PL(unmatchdMapAdjustmentProc3Injector)
				+ PL(unmatchdMapAdjustmentProc4Injector)
				+ PL(unmatchdMapAdjustmentProc5Injector);
		}
	} mapAdjustment;

	union {
		DllErrorCode code;
		struct {
			bool unmatchdMapJustifyProc1Injector : 1;
			bool unmatchdMapJustifyProc2Injector : 1;
			bool unmatchdMapJustifyProc3Injector : 1;
			bool unmatchdMapJustifyProc4Injector : 1;
		};

		std::string print() {
			return PL(unmatchdMapJustifyProc1Injector)
				+ PL(unmatchdMapJustifyProc2Injector)
				+ PL(unmatchdMapJustifyProc3Injector)
				+ PL(unmatchdMapJustifyProc4Injector);
		}
	} mapJustify;

	union {
		DllErrorCode code;
		struct {
			bool unmatchdMapNudgeViewProc1Injector : 1;
		};

		std::string print() {
			return PL(unmatchdMapNudgeViewProc1Injector);
		}
	} mapNudge;

	union {
		DllErrorCode code;
		struct {
			bool unmatchdMapPopupProc1Injector : 1;
			bool unmatchdMapPopupProc2Injector : 1;
			bool unmatchdMapPopupProc3Injector : 1;
		};

		std::string print() {
			return PL(unmatchdMapPopupProc1Injector)
				+ PL(unmatchdMapPopupProc2Injector)
				+ PL(unmatchdMapPopupProc3Injector);
		}
	} mapPopup;

	union {
		DllErrorCode code;
		struct {
			bool unmatchdMapViewProc1Injector : 1;
			bool unmatchdMapViewProc2Injector : 1;
			bool unmatchdMapViewProc3Injector : 1;
			bool unmatchdMapViewProc4Injector : 1;
		};

		std::string print() {
			return PL(unmatchdMapViewProc1Injector)
				+ PL(unmatchdMapViewProc2Injector)
				+ PL(unmatchdMapViewProc3Injector)
				+ PL(unmatchdMapViewProc4Injector);
		}

	} mapView;

	union {
		DllErrorCode code;
		struct {
			bool unmatchdDebugProc1Injector : 1;
		};

		std::string print() {
			return PL(unmatchdDebugProc1Injector);
		}
	} debug;

	union {
		DllErrorCode code;
		struct {
			bool unmatchdTooltipAndButtonProc1Injector : 1;
			bool unmatchdTooltipAndButtonProc2Injector : 1;
			bool unmatchdTooltipAndButtonProc3Injector : 1;
			bool unmatchdTooltipAndButtonProc4Injector : 1;
			bool unmatchdTooltipAndButtonProc5Injector : 1;
			bool unmatchdTooltipAndButtonProc6Injector : 1;
			bool unmatchdTooltipAndButtonProc7Injector : 1;
			bool unmatchdTooltipAndButtonProc8Injector : 1;
			bool unmatchdTooltipAndButtonProc9Injector : 1;
			bool unmatchdTooltipAndButtonProc10Injector : 1;
		};

		std::string print() {
			return PL(unmatchdTooltipAndButtonProc1Injector)
				+ PL(unmatchdTooltipAndButtonProc2Injector)
				+ PL(unmatchdTooltipAndButtonProc3Injector)
				+ PL(unmatchdTooltipAndButtonProc4Injector)
				+ PL(unmatchdTooltipAndButtonProc5Injector)
				+ PL(unmatchdTooltipAndButtonProc6Injector)
				+ PL(unmatchdTooltipAndButtonProc7Injector)
				+ PL(unmatchdTooltipAndButtonProc8Injector)
				+ PL(unmatchdTooltipAndButtonProc9Injector)
				+ PL(unmatchdTooltipAndButtonProc10Injector);
		}
	} tooltipAndButton;

	union {
		DllErrorCode code;
		struct {
			bool unmatchdCBitmapFontProc1Injector : 1;
			bool unmatchdCBitmapFontProc2Injector : 1;
		};

		std::string print() {
			return PL(unmatchdCBitmapFontProc1Injector)
				 + PL(unmatchdCBitmapFontProc2Injector);
		}
	} cBitmapFont;

	void operator |= (DllError e)
	{
		this->date.code |= e.date.code;
		this->eventDialog.code |= e.eventDialog.code;
		this->fileSave.code |= e.fileSave.code;
		this->font.code |= e.font.code;
		this->ime.code |= e.ime.code;
		this->input.code |= e.input.code;
		this->listFiledAdjustment.code |= e.listFiledAdjustment.code;
		this->localization.code |= e.localization.code;
		this->mainText.code |= e.mainText.code;
		this->mapAdjustment.code |= e.mapAdjustment.code;
		this->mapJustify.code |= e.mapJustify.code;
		this->mapNudge.code |= e.mapNudge.code;
		this->mapPopup.code |= e.mapPopup.code;
		this->mapView.code |= e.mapView.code;
		this->tooltipAndButton.code |= e.tooltipAndButton.code;
		this->debug.code |= e.debug.code;
		this->cBitmapFont.code |= e.cBitmapFont.code;
	}

	bool errorCheck() {
		return this->date.code > 0
			|| this->eventDialog.code > 0
			|| this->fileSave.code > 0
			|| this->font.code > 0
			|| this->ime.code > 0
			|| this->input.code > 0
			|| this->listFiledAdjustment.code > 0
			|| this->localization.code > 0
			|| this->mainText.code > 0
			|| this->mapAdjustment.code > 0
			|| this->mapJustify.code > 0
			|| this->mapNudge.code > 0
			|| this->mapPopup.code > 0
			|| this->mapView.code > 0
			|| this->tooltipAndButton.code > 0
			|| this->debug.code > 0
			|| this->cBitmapFont.code > 0;
	}

	std::string print() {
		return this->tooltipAndButton.print()
			+ "--------------\n"
			+ this->mapView.print()
			+ "--------------\n"
			+ this->debug.print()
			+ "--------------\n"
			+ this->mapPopup.print()
			+ "--------------\n"
			+ this->mapNudge.print()
			+ "--------------\n"
			+ this->mapJustify.print()
			+ "--------------\n"
			+ this->mapAdjustment.print()
			+ "--------------\n"
			+ this->mainText.print()
			+ "--------------\n"
			+ this->localization.print()
			+ "--------------\n"
			+ this->listFiledAdjustment.print()
			+ "--------------\n"
			+ this->input.print()
			+ "--------------\n"
			+ this->ime.print()
			+ "--------------\n"
			+ this->font.print()
			+ "--------------\n"
			+ this->fileSave.print()
			+ "--------------\n"
			+ this->eventDialog.print()
			+ "--------------\n"
			+ this->date.print()
			+ "--------------\n"
			+ this->cBitmapFont.print();
	}
};

// same std::basic_string<char>
typedef struct _ParadoxTextObject {
	union {
		char text[0x10];
		char* p;
	} t;
	UINT64 len;
	UINT64 len2;

	std::string getString() {
		if (len2 >= 0x10) {
			return std::string(t.p);
		}
		else {
			return std::string(t.text);
		}
	}

	void setString(std::string *src) {

		if (len2 >= 0x10) {
			auto hHeap = GetProcessHeap();
			HeapFree(hHeap, NULL, t.p);
		}

		len2 = src->capacity();
		len = src->length();

		if (len >= 0x10) {
 			len2 = 0xFF;
			auto hHeap = GetProcessHeap();
			auto p = (char*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY,  0xFF);
			if (p != NULL) {
				memcpy(p, src->c_str(), len+1);
				t.p = p;
			}
		}
		else {
			memcpy(t.text, src->c_str(), len+1);
		}
	}

} ParadoxTextObject;

typedef struct _RunOptions {
	bool test;
	bool reversingWordsBattleOfArea;
	int separateCharacterCodePoint;
	int lineBreakBufferWidth;
} RunOptions;

namespace Version {
	bool DetectVersion();
}

namespace Ini {
	void GetOptionsFromIni(RunOptions *option);
}

namespace Debug {
	DllError Init(RunOptions option);
}

namespace Font {
	DllError Init(RunOptions option);
}

namespace MainText {
	DllError Init(RunOptions option);
}

namespace TooltipAndButton {
	DllError Init(RunOptions option);
}

namespace MapView {
	DllError Init(RunOptions option);
}

namespace MapAdjustment {
	DllError Init(RunOptions option);
}

namespace MapJustify {
	DllError Init(RunOptions option);
}

namespace EventDialog {
	DllError Init(RunOptions option);
}

namespace MapPopup {
	DllError Init(RunOptions option);
}

namespace ListFieldAdjustment {
	DllError Init(RunOptions option);
}

namespace Validator {
	void Validate(DllError dllError, RunOptions options);
	bool ValidateVersion();
}

namespace FileSave {
	DllError Init(RunOptions option);
}

namespace Date {
	DllError Init(RunOptions option);
}

namespace MapNudgeView {
	DllError Init(RunOptions option);
}

namespace Ime {
	DllError Init(RunOptions option);
}

namespace Input {
	DllError Init(RunOptions option);
}

namespace Localization {
	DllError Init(RunOptions option);
}

namespace CBitmapFont {
	DllError Init(RunOptions option);
}
