#include "plugin.hpp"

struct Lilac : Module {
  enum ParamIds {
    MODE_TOGGLE_PARAM,
    AFTER_RECORD_PARAM,
    STOP_BUTTON_PARAM,
    ERASE_BUTTON_PARAM,
    NUM_PARAMS,
  };
  enum InputIds {
    MODE_CV_INPUT,
    STOP_CV_INPUT,
    ERASE_CV_INPUT,
    MAIN_INPUT,
    NUM_INPUTS,
  };
  enum OutputIds {
    MAIN_OUTPUT,
    NUM_OUTPUTS,
  };
  enum LightIds {
    RECORD_STATUS_LIGHT,
    OVERDUB_STATUS_LIGHT,
    PLAY_STATUS_LIGHT,
    STOP_STATUS_LIGHT,
    NUM_LIGHTS,
  };

  enum Modes {
    STOPPED,
    RECORDING,
    OVERDUBBING,
    PLAYING,
  };

  std::vector<float> loop;
  int mode;
  unsigned int position;
  float tracking;
  dsp::BooleanTrigger modeTrigger;
  dsp::BooleanTrigger eraseTrigger;
  dsp::BooleanTrigger stopTrigger;
  dsp::SlewLimiter trackingSmoother;
  dsp::SlewLimiter playbackSmoother;
  dsp::ClockDivider lightDivider;
  dsp::ClockDivider logDivider;

  Lilac() {
    config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    configParam(MODE_TOGGLE_PARAM, 0.f, 1.f, 0.f, "");
    configParam(AFTER_RECORD_PARAM, 0.f, 1.f, 0.f, "");
    configParam(STOP_BUTTON_PARAM, 0.f, 1.f, 0.f, "");
    configParam(ERASE_BUTTON_PARAM, 0.f, 1.f, 0.f, "");
    mode = STOPPED;
    position = 0;
    tracking = 0.f;
    trackingSmoother.rise = 400.0f;
    trackingSmoother.fall = 400.0f;
    playbackSmoother.rise = 400.0f;
    playbackSmoother.fall = 400.0f;
    lightDivider.setDivision(pow(2, 9));
    logDivider.setDivision(pow(2, 13));
  }

  void process(const ProcessArgs &args) override {
    bool toggleTriggered = modeTrigger.process(params[MODE_TOGGLE_PARAM].getValue() + inputs[MODE_CV_INPUT].getVoltage() > 0.f);
    bool stopTriggered = stopTrigger.process(params[STOP_BUTTON_PARAM].getValue() + inputs[STOP_CV_INPUT].getVoltage() > 0.f);
    bool eraseTriggered = eraseTrigger.process(params[ERASE_BUTTON_PARAM].getValue() + inputs[ERASE_CV_INPUT].getVoltage() > 0.f);
    bool overdubAfterRecord = params[AFTER_RECORD_PARAM].getValue() > 0.f;

    if (toggleTriggered) {
      if (mode == STOPPED && loop.empty()) {
        mode = RECORDING;
      } else if (mode == STOPPED && !loop.empty()) {
        position = 0;
        mode = PLAYING;
      } else if (mode == RECORDING) {
        if (overdubAfterRecord) {
          mode = OVERDUBBING;
        } else {
          mode = PLAYING;
        }
      } else if (mode == OVERDUBBING) {
        mode = PLAYING;
      } else if (mode == PLAYING) {
        mode = OVERDUBBING;
      }
    }

    if (stopTriggered) {
      mode = STOPPED;
    }

    if (eraseTriggered) {
      mode = STOPPED;
      loop.clear();
      position = 0;
      outputs[MAIN_OUTPUT].setVoltage(0.f);
    }

    float trackingGate = mode == RECORDING || mode == OVERDUBBING ? 1.0f : 0.0f;
    float trackingEnv = trackingSmoother.process(args.sampleTime, trackingGate);
    float playbackGate = mode == STOPPED ? 0.0f : 1.0f;
    float playbackEnv = playbackSmoother.process(args.sampleTime, playbackGate);

    if (mode == RECORDING) {
      loop.push_back(0.f);
    }

    if (!loop.empty()) {
      if (position == loop.size()) {
        position = 0;
      }
      outputs[MAIN_OUTPUT].setVoltage(playbackEnv * loop[position]);
      loop[position] += trackingEnv * inputs[MAIN_INPUT].getVoltage();
      ++position;
    }

    if (lightDivider.process()) {
      lights[RECORD_STATUS_LIGHT].value = 0.0;
      lights[PLAY_STATUS_LIGHT].value = 0.0;
      lights[OVERDUB_STATUS_LIGHT].value = 0.0;
      lights[STOP_STATUS_LIGHT].value = 0.0;
      if (mode == PLAYING) {
        lights[PLAY_STATUS_LIGHT].value = 1.0;
      }
      if (mode == RECORDING) {
        lights[RECORD_STATUS_LIGHT].value = 1.0;
      }
      if (mode == OVERDUBBING) {
        lights[OVERDUB_STATUS_LIGHT].value = 1.0;
      }
      if (mode == STOPPED && !loop.empty()) {
        lights[STOP_STATUS_LIGHT].value = 1.0;
      }
    }
  }
};

struct LilacWidget : ModuleWidget {
  LilacWidget(Lilac *module) {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Lilac.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<CKD6>(mm2px(Vec(21.967, 26.937)), module, Lilac::MODE_TOGGLE_PARAM));
    addParam(createParam<CKSS>(mm2px(Vec(20.723, 44.214)), module, Lilac::AFTER_RECORD_PARAM));
    addParam(createParamCentered<CKD6>(mm2px(Vec(21.967, 69.484)), module, Lilac::STOP_BUTTON_PARAM));
    addParam(createParamCentered<CKD6>(mm2px(Vec(21.951, 87.957)), module, Lilac::ERASE_BUTTON_PARAM));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.984, 26.937)), module, Lilac::MODE_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.984, 69.484)), module, Lilac::STOP_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.0, 88.005)), module, Lilac::ERASE_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.98, 112.341)), module, Lilac::MAIN_INPUT));

    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.462, 112.3)), module, Lilac::MAIN_OUTPUT));

    addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(6.058, 42.67)), module, Lilac::RECORD_STATUS_LIGHT));
    addChild(createLightCentered<MediumLight<YellowLight>>(mm2px(Vec(12.938, 42.67)), module, Lilac::OVERDUB_STATUS_LIGHT));
    addChild(createLightCentered<MediumLight<BlueLight>>(mm2px(Vec(6.058, 49.549)), module, Lilac::STOP_STATUS_LIGHT));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(12.938, 49.549)), module, Lilac::PLAY_STATUS_LIGHT));
  }
};

Model *modelLilac = createModel<Lilac, LilacWidget>("Lilac");
