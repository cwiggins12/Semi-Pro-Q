
#pragma once

//==============================================================================
/** Config values for parameters, filters, and types, along with the string arrays that they reference
*/
//min and max gain and freq values
inline constexpr float MIN_FREQ = 20.0f;
inline constexpr float MID_FREQ = 1000.0f;
inline constexpr float MAX_FREQ = 20000.0f;
inline constexpr float NEG_INF_DB = -80.0f;
inline constexpr float MIN_ANALYSIS_DB = -96.0f;
inline constexpr float MIN_DB = -72.0f;
inline constexpr float MAX_DB = 24.0f;
//eq and params amount
inline constexpr int MAX_FILTERS = 12;
inline constexpr int PARAMS_PER_FILTER = 6;
//indices of filter parameters
inline constexpr int FREQ = 0;
inline constexpr int GAIN = 1;
inline constexpr int QUALITY = 2;
inline constexpr int TYPE = 3;
inline constexpr int B_WORTH = 4;
inline constexpr int BYPASS = 5;
//gain ramp for smoothing
inline constexpr float GAIN_RAMP_TIME = 0.1f;
//indices of non filter parameters/properties
inline constexpr int PREGAIN = MAX_FILTERS * PARAMS_PER_FILTER;
inline constexpr int POSTGAIN = PREGAIN + 1;
inline constexpr int ANALYSER_ON = 0 + MAX_FILTERS;
inline constexpr int ANALYSER_MODE = 1 + MAX_FILTERS;
inline constexpr int PEAK_ON = 2 + MAX_FILTERS;
inline constexpr int PEAK_MODE = 3 + MAX_FILTERS;
inline constexpr int MINIMIZE_GAIN = 4 + MAX_FILTERS;
inline constexpr int MINIMIZE_SELECTED = 5 + MAX_FILTERS;
inline constexpr int MINIMIZE_SETTINGS = 6 + MAX_FILTERS;
inline constexpr int SELECTED_FILTER = 7 + MAX_FILTERS;
inline constexpr int SELECTED_X = 8 + MAX_FILTERS;
inline constexpr int SELECTED_Y = 9 + MAX_FILTERS;
inline constexpr int GAIN_X = 10 + MAX_FILTERS;
inline constexpr int GAIN_Y = 11 + MAX_FILTERS;
inline constexpr int SETTINGS_X = 12 + MAX_FILTERS;
inline constexpr int SETTINGS_Y = 13 + MAX_FILTERS;
//filter coefficient specific variables
//2nd order has 6 but juce internally filters out one of them(a0)
inline constexpr int COEFF_SIZE = 6 - 1;
inline constexpr int FILTER_ORDER = 2;
inline constexpr int MAX_STAGES = 4;
inline constexpr float COEFF_RAMP_TIME = 0.012f;
//filter types
inline constexpr int PEAK = 0;
inline constexpr int HIGHPASS_OCT = 1;
inline constexpr int LOWPASS_OCT = 2;
inline constexpr int HIGHPASS_Q = 3;
inline constexpr int LOWPASS_Q = 4;
inline constexpr int HIGHSHELF = 5;
inline constexpr int LOWSHELF = 6;
inline constexpr int NOTCH = 7;
//front end timings
inline constexpr int TOOLTIP_DELAY_MS = 200;
inline constexpr int TIMER_FPS = 30;
//fft/analyser configs
//would love for this to be 14, or at least get a better way to deal with the low end
inline constexpr int FFT_ORDER = 13;
inline constexpr int FFT_SIZE = 1 << FFT_ORDER;
inline constexpr int FFT_HOP_AMT = 4;
inline constexpr int FFT_HOP_SIZE = FFT_SIZE / FFT_HOP_AMT;
inline constexpr int FFT_BIN_AMT = FFT_SIZE / 2 + 1;
inline constexpr float FFT_SLOPE = 4.5f;
inline constexpr int FFT_ATTACK = 20;
inline constexpr int FFT_RELEASE = 3;
//peak configs
inline constexpr float PEAK_DECAY_TIME = 0.2f;
inline constexpr int PEAK_HOLD_FRAMES = TIMER_FPS;
inline constexpr float EST_CLIP_BARRIER = 0.98f;
inline constexpr int UPWARD_ATTACK_MULTIPLIER = 15;
//pixel configs for front end
//window dims
inline constexpr int MAIN_WINDOW_WIDTH = 1200;
inline constexpr int MAIN_WINDOW_HEIGHT = 675;
inline constexpr int CREDIT_WINDOW_WIDTH = 900;
inline constexpr int CREDIT_WINDOW_HEIGHT = 500;
inline constexpr int HELP_WINDOW_WIDTH = 1020;
inline constexpr int HELP_WINDOW_HEIGHT = 650;
inline constexpr int METER_AREA_WIDTH = MAIN_WINDOW_WIDTH / 16;
//button dims
inline constexpr int MINIMIZE_BUTTON_DIM = 20;
inline constexpr int DRAG_BUTTON_DIM = 20;
inline constexpr int HELP_CREDITS_BUTTON_W = 50;
inline constexpr int HELP_CREDITS_BUTTON_H = 30;
//label/textbox
inline constexpr int LABEL_HEIGHT = 20;
inline constexpr int TEXTBOX_HEIGHT = 18;
inline constexpr int TEXTBOX_WIDTH = 100;
//spacers
inline constexpr int BORDER_SPACING = 16;
inline constexpr int BUTTON_SPACING = 4;
inline constexpr int METER_SPACING = 5;
inline constexpr int PARAM_BUTTON_SPACING = 6;
inline constexpr int METER_BORDER_SPACING = 10;
//font sizes
inline constexpr int BG_FONT_SIZE = 11;
inline constexpr int PEAK_FONT_SIZE = 10;
inline constexpr float LABEL_FONT_SIZE = 14.0f;
//div lines in comps, rounded corner size
inline constexpr float DIV_LINE_WIDTH = 2.0f;
inline constexpr float CORNER_SIZE = 4.0f;
inline constexpr float BG_CORNER_SIZE = 5.0f;
inline constexpr float OUTLINE_WIDTH = 2.0f;
inline constexpr float COLOR_CHANGE = 0.2f;
//peak and meter dims
inline constexpr int METER_WIDTH = 20;
inline constexpr int PEAK_CLIP_HEIGHT = 4;
//Size and Top left for minimizable components
inline constexpr int GAIN_SIZE_X = 140;
inline constexpr int GAIN_SIZE_Y = 120;
inline constexpr int GAIN_TOPLEFT_X = 945;
inline constexpr int GAIN_TOPLEFT_Y = 520;
inline constexpr int SETTINGS_SIZE_X = 210;
inline constexpr int SETTINGS_SIZE_Y = 120;
inline constexpr int SETTINGS_TOPLEFT_X = 25;
inline constexpr int SETTINGS_TOPLEFT_Y = 520;
inline constexpr int SELECTED_SIZE_X = 210;
inline constexpr int SELECTED_SIZE_Y = 220;
inline constexpr int SELECTED_TOPLEFT_X = 425;
inline constexpr int SELECTED_TOPLEFT_Y = 420;
inline constexpr int FADER_START_Y = LABEL_HEIGHT * 2;

//const strings for parameter names for automation, declared as extern in header to be used everywhere
inline juce::StringArray params{ "1Freq", "1Gain", "1Quality", "1Type", "1dB/Oct", "1Bypass",
                                 "2Freq", "2Gain", "2Quality", "2Type", "2dB/Oct", "2Bypass",
                                 "3Freq", "3Gain", "3Quality", "3Type", "3dB/Oct", "3Bypass",
                                 "4Freq", "4Gain", "4Quality", "4Type", "4dB/Oct", "4Bypass",
                                 "5Freq", "5Gain", "5Quality", "5Type", "5dB/Oct", "5Bypass",
                                 "6Freq", "6Gain", "6Quality", "6Type", "6dB/Oct", "6Bypass",
                                 "7Freq", "7Gain", "7Quality", "7Type", "7dB/Oct", "7Bypass",
                                 "8Freq", "8Gain", "8Quality", "8Type", "8dB/Oct", "8Bypass",
                                 "9Freq", "9Gain", "9Quality", "9Type", "9dB/Oct", "9Bypass",
                                 "10Freq", "10Gain", "10Quality", "10Type", "10dB/Oct", "10Bypass",
                                 "11Freq", "11Gain", "11Quality", "11Type", "11dB/Oct", "11Bypass",
                                 "12Freq", "12Gain", "12Quality", "12Type", "12dB/Oct", "12Bypass",
                                 "PreGain", "PostGain" };
//property names to call easily when dealing with value tree
inline juce::StringArray props{ "1Init", "2Init", "3Init", "4Init", "5Init", "6Init", "7Init", "8Init", "9Init", "10Init", "11Init", "12Init",
                                "analyserOn", "analyserMode", "peakOn", "peakMode", "minimizeGain", "minimizeSelectedFilter", "minimizeConfigs", 
                                "selectedFilter", "selectedX", "selectedY", "gainX", "gainY", "settingsX", "settingsY" };
//eq filter type list and butterworth dB/octave lists for audio parameter choices, only used by the processor
inline juce::StringArray filterTypes{ "PEAK", "HI-PASS\n(dB/OCT)", "LO-PASS\n(dB/OCT)", "HI-PASS\n(Q)", "LO-PASS\n(Q)", "HI-SHLF", "LO-SHLF", "NOTCH" };
inline juce::StringArray b_worths{ "12dB/OCT", "24dB/OCT", "36dB/OCT", "48dB/OCT" };

//==============================================================================
/** Formatting and range templates
*/
//template to have true logarithmic skew for frequency slider: returns start, end, from0to1, To0to1
template <typename ValueT>
juce::NormalisableRange<ValueT> logRange(ValueT min, ValueT max) {
    ValueT rng{ std::log(max / min) };
    return { min, max,
        [=](ValueT min, ValueT, ValueT v) { return std::exp(v * rng) * min; },
        [=](ValueT min, ValueT, ValueT v) { return std::log(v / min) / rng; }
    };
}

//string formatting for freq textboxes
inline static juce::String formatFrequency(float value, int decimalsHz = 1, int decimalsKHz = 2) {
    if (value < 1000.0f) {
        return juce::String(value, decimalsHz) + " Hz";
    }
    return juce::String(value / 1000.0f, decimalsKHz) + " kHz";
}

//string formatting for gain textboxes
inline static juce::String formatGain(float value, int decimals = 2) {
    return juce::String(value, decimals) + " dB";
}

//string formatting for quality textboxes
inline static juce::String formatQuality(float value, int decimals = 2) {
    return juce::String(value, decimals);
}

//precompute normalisableRange for front end reference
inline static juce::NormalisableRange<float> freqRange{ logRange<float>(MIN_FREQ, MAX_FREQ) };
