#include <QApplication>
#include "demo.h"
#include "subject.h"
#include "list.h"
#include "player.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    Map map;

    SubjectRussia* r1 = new SubjectRussia();
    r1->add_coord({0.1, 0.1});
    r1->add_coord({0.4, 0.1});
    r1->add_coord({0.4, 0.4});
    r1->add_coord({0.1, 0.4});

    SubjectRussia* r2 = new SubjectRussia();
    r2->add_coord({0.5, 0.2});
    r2->add_coord({0.8, 0.2});
    r2->add_coord({0.8, 0.5});
    r2->add_coord({0.5, 0.5});
    r2->visit();

    map.get_subjects().push(r1);
    map.get_subjects().push(r2);

    for (int i = 0; i < 10000; i++){
        SubjectRussia* r2 = new SubjectRussia();
        r2->add_coord({(double) rand()/RAND_MAX, (double) rand()/RAND_MAX});
        r2->add_coord({(double) rand()/RAND_MAX, (double) rand()/RAND_MAX});
        r2->add_coord({(double) rand()/RAND_MAX, (double) rand()/RAND_MAX});
        map.get_subjects().push(r2);
    }
    MapWidget mapWidget(&map);
    mapWidget.resize(800, 600);
    mapWidget.show();


    return app.exec();

}