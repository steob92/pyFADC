#ifndef FADCSettings_H
#define FADCSettings_H
struct FADCSettings{

  int pedmin;
  int pedmax;
  int pulsemin;
  int pulsemax;
  int nbrSamples;
  double channel_coupling;
  double delayTime;
  double fullScale;
  double trigCoupling;
  double trigSlope;
  double trigLevelmv;
  double sampInterval;
  double offset;
  double startX;
  double startY;
  double sizeX;
  double sizeY;
  double stepSizeX;
  double stepSizeY;
};
#endif
