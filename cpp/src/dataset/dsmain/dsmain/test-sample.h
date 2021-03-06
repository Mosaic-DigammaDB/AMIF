
//           Copyright Nathaniel Christen 2019.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           http://www.boost.org/LICENSE_1_0.txt)


#ifndef TEST_SAMPLE__H
#define TEST_SAMPLE__H

#include "kans.h"
#include "accessors.h"

#include "range-int.h"
#include "precon.h"

#include "posit-lib/posit.h"


#include <QtGlobal>
#include <QString>
#include <QVector>

KANS_(DSM)


class Test_Sample
{
 int index_;

 typedef Range_Int<qint32, -10000, 10000> temperature_adj_t;
 temperature_adj_t temperature_adj_;

 typedef Range_Int<qint8, 0, 100> oxy_t;
 oxy_t oxy_;

 Posit flow_;
 Posit time_with_flow_;
 Posit time_against_flow_;
 Posit temperature_kelvin_;
 Posit average_time_;
 Posit delta_time_;


public:

 Test_Sample(int index);

 ACCESSORS(int ,index)
 ACCESSORS(Posit ,flow)
 ACCESSORS(Posit ,time_with_flow)
 ACCESSORS(Posit ,time_against_flow)
 ACCESSORS(Posit ,temperature_kelvin)
 ACCESSORS(Posit ,average_time)
 ACCESSORS(Posit ,delta_time)

 ACCESSORS__GET__SDECLARE(qint16 ,temperature_adj)
 ACCESSORS__GET__SDECLARE(quint8 ,oxy)

 void set_temperature_adj(precon_pr<temperature_adj_t> pr);
 void set_temperature_adj(qint16 q, precon<temperature_adj_t> pre);

 void set_oxy(precon_pr<oxy_t> pr);
 void set_oxy(quint8 q, precon<oxy_t> pre);

 void finalize();

 static void read_samples_from_file(QString path, QVector<Test_Sample*>& samps);
 static void read_samples_from_raw_file(QString path, QVector<Test_Sample*>& samps);
 static void write_samples_to_file(QString path, QVector<Test_Sample*>& samps);
};


_KANS(DSM)

#endif // TEST_SAMPLE__H
