//#pragma once

//#include <QQueue>
//#include <QPair>
//#include <QMutex>
//#include <QMutexLocker>

//#include <QRunnable>
//#include <QThreadPool>
//#include "utils/rgba.h"

//// Forward declaration
//class RayTracer;
//class RayTraceScene;

//class RaytraceTask : public QRunnable
//{
//public:
//    RaytraceTask(RayTracer* raytracer,
//                 RGBA* imageData,
//                 const RayTraceScene& scene,
//                 int startRow, int endRow)
//        : m_raytracer(raytracer),
//        m_imageData(imageData),
//        m_scene(scene),
//        m_startRow(startRow),
//        m_endRow(endRow)
//    {}

//    void run() override;  // This is the modified line

//private:
//    RayTracer* m_raytracer;
//    RGBA* m_imageData;
//    const RayTraceScene& m_scene;
//    int m_startRow;
//    int m_endRow;
//};

