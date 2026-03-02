#ifndef PROJECT_H
#define PROJECT_H

#include "harmonicsolver.h"

namespace Backend::Core
{

class Response;

struct Project
{
    Project();
    Project(QString const& uName);
    ~Project() = default;

    void clear();
    bool write(QString const& pathFile);

    QString name;
    QList<Response> responses;
    QList<Response> spectrums;
    HarmonicOptions options;
    HarmonicSolution solution;
};

}

#endif // PROJECT_H
