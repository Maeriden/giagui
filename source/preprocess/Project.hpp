#ifndef GIAGUI_PREPROCESS_PROJECT_HPP_
#define GIAGUI_PREPROCESS_PROJECT_HPP_ 1

#include <QDir>


namespace poglar {

  class Project {
  public:
    QString errorMessage;
    
    Project(const QDir &path);
    bool export_to(const QDir &destination);

  private:
    QDir path_;
  };

} /* namespace poglar */

#endif /* GIAGUI_PREPROCESS_PROJECT_HPP_ */
