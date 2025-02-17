struct LooperOneWidget : LooperWidget {
  LooperOneWidget(Looper *module) {
    setModule(module);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Looper.svg")));

    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

    addParam(createParamCentered<LargeWarmButton>(mm2px(Vec(12.7, 21.135)), module, Looper::MODE_TOGGLE_PARAM));
    addParam(createParamCentered<WarmButton>(mm2px(Vec(18.266, 49.817)), module, Looper::STOP_BUTTON_PARAM));
    addParam(createParamCentered<WarmButton>(mm2px(Vec(18.47, 65.473)), module, Looper::ERASE_BUTTON_PARAM));
    addParam(createParamCentered<WarmKnob>(mm2px(Vec(18.47, 81.129)), module, Looper::MIX_PARAM));

    addInput(createInputCentered<LilacPort>(mm2px(Vec(6.658, 34.16)), module, Looper::MODE_CV_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(6.658, 49.817)), module, Looper::STOP_CV_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(6.658, 65.473)), module, Looper::ERASE_CV_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(6.658, 81.129)), module, Looper::MIX_CV_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(6.658, 97.091)), module, Looper::MAIN_1_INPUT));
    addInput(createInputCentered<LilacPort>(mm2px(Vec(18.758, 97.091)), module, Looper::MAIN_2_INPUT));

    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(6.666, 112.441)), module, Looper::MAIN_1_OUTPUT));
    addOutput(createOutputCentered<LilacPort>(mm2px(Vec(18.766, 112.441)), module, Looper::MAIN_2_OUTPUT));

    addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(15.978, 35.219)), module, Looper::RECORD_STATUS_LIGHT));
    addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(20.651, 35.219)), module, Looper::PLAY_STATUS_LIGHT));
  }
};
