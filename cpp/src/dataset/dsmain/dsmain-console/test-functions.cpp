
//           Copyright Nathaniel Christen 2019.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           http://www.boost.org/LICENSE_1_0.txt)

#include "defines.h"


#ifdef USING_KPH

#include "test-functions.h"

#include "kauvir-code-model/kauvir-code-model.h"

#include "kauvir-code-model/kcm-channel-group.h"

#include "kauvir-type-system/kauvir-type-system.h"

#include "kauvir-code-model/kcm-callable-value.h"

#include "kcm-command-runtime/kcm-command-runtime-table.h"

#include "kcm-command-runtime/kcm-command-runtime-router.h"

#include "PhaonLib/phaon-channel-group-table.h"
#include "PhaonLib/phaon-symbol-scope.h"
#include "PhaonLib/phaon-function.h"

#include "ScignStage-tree-table/scignstage-tree-table-dialog.h"

#include "application-model/application-model.h"


#include <QTextStream>

#include <QDebug>

#include <QEventLoop>

#undef slots
#include "kcm-lisp-bridge/kcm-lisp-eval.h"

USING_KANS(KCL)
USING_KANS(PhaonLib)

KANS_(Phaon)


void* insert_envv(void* kind, void* test)
{
 static QMap<QString, void*> hold;
 QString* k = reinterpret_cast<QString*>(kind);
 if(test)
 {
  void** pv = new void*;
  *pv = test;
  hold[*k] = pv;
 }
 return hold.value(*k);
}

void* p_envv(void* kind)
{
 if(kind)
 {
  qDebug() << "Kind: " << *(QString*)kind;
  return insert_envv(kind, nullptr);
 }
 else
 {
  qDebug() << "In envv: Return kind could not be determined.";
  return nullptr;
 }
}

void* envv(void* kind)
{
 if(kind)
 {
  qDebug() << "Kind: " << *(QString*)kind;
  void* pv = insert_envv(kind, nullptr);
  return *(void**)(pv);
 }
 else
 {
  qDebug() << "In envv: Return kind could not be determined.";
  return nullptr;
 }
}

void test_0_ss(QString s1, QString s2)
{
 qDebug() << "s1 = " << s1 << "s2 = " << s2;
}

QString test_s_ss(QString s1, QString s2)
{
 qDebug() << "s1 = " << s1 << "s2 = " << s2;
 qDebug() << "returning: s_ss";
 return "s_ss";
}

void test_msgbox(ScignStage_Tree_Table_Dialog* dlg, QString msg)
{
 dlg->test_msgbox(msg);
}

void show_graphic(ScignStage_Tree_Table_Dialog* dlg, QString code)
{
 if(Application_Model* appm = static_cast<Application_Model*>(dlg->application_model()))
 {
  appm->show_graphic(dlg, code);
 }
}

void hide_graphic(ScignStage_Tree_Table_Dialog* dlg, QString code)
{
 if(Application_Model* appm = static_cast<Application_Model*>(dlg->application_model()))
 {
  appm->hide_graphic(dlg, code);
 }
}

void expand_sample(ScignStage_Tree_Table_Dialog* dlg, int index)
{
 if(Application_Model* appm = static_cast<Application_Model*>(dlg->application_model()))
 {
  appm->expand_sample(dlg, index);
 }
}

void copy_column(ScignStage_Tree_Table_Dialog* dlg, QString code)
{
 if(Application_Model* appm = static_cast<Application_Model*>(dlg->application_model()))
 {
  appm->copy_column(dlg, code);
 }
}

void init_test_functions(void* origin, Kauvir_Code_Model& kcm,
  Phaon_Channel_Group_Table& table, Phaon_Symbol_Scope& pss)
{
 QString* satypename = new QString("ScignStage_Tree_Table_Dialog*");
 insert_envv(satypename, origin);

 kcm.set_envv_fn(&p_envv);

 Kauvir_Type_System* type_system = kcm.type_system();

 KCM_Channel_Group g1(kcm.channel_names());
 {
  g1.add_lambda_carrier(
    {kcm.get_kcm_type_by_kauvir_type_object( &type_system->type_object__str() ), nullptr},
     QString()
    );

  g1.add_lambda_carrier(
    {kcm.get_kcm_type_by_kauvir_type_object( &type_system->type_object__str() ), nullptr},
     QString()
    );

  table.init_phaon_function(g1, pss, "test_0_ss", 700, &test_0_ss);

  g1.clear_all();
 }

 {
  g1.add_sigma_carrier(
    {kcm.get_kcm_type_by_type_name("ScignStage_Tree_Table_Dialog*"), nullptr},
     QString()
    );

  g1.add_lambda_carrier(
    {kcm.get_kcm_type_by_kauvir_type_object( &type_system->type_object__str() ), nullptr},
     QString()
    );

  table.init_phaon_function(g1, pss, "test_msgbox", 710, &test_msgbox);

  g1.clear_all();
 }

 {
  g1.add_sigma_carrier(
    {kcm.get_kcm_type_by_type_name("ScignStage_Tree_Table_Dialog*"), nullptr},
     QString()
    );

  g1.add_lambda_carrier(
    {kcm.get_kcm_type_by_kauvir_type_object( &type_system->type_object__str() ), nullptr},
     QString()
    );

  table.init_phaon_function(g1, pss, "show_graphic", 710, &show_graphic);

  g1.clear_all();
 }

 {
  g1.add_sigma_carrier(
    {kcm.get_kcm_type_by_type_name("ScignStage_Tree_Table_Dialog*"), nullptr},
     QString()
    );

  g1.add_lambda_carrier(
    {kcm.get_kcm_type_by_kauvir_type_object( &type_system->type_object__str() ), nullptr},
     QString()
    );

  table.init_phaon_function(g1, pss, "hide_graphic", 710, &hide_graphic);

  g1.clear_all();
 }


 {
  g1.add_sigma_carrier(
    {kcm.get_kcm_type_by_type_name("ScignStage_Tree_Table_Dialog*"), nullptr},
     QString()
    );

  g1.add_lambda_carrier(
    {kcm.get_kcm_type_by_kauvir_type_object( &type_system->type_object__str() ), nullptr},
     QString()
    );

  table.init_phaon_function(g1, pss, "copy_column", 710, &copy_column);

  g1.clear_all();
 }

 {
  g1.add_sigma_carrier(
    {kcm.get_kcm_type_by_type_name("ScignStage_Tree_Table_Dialog*"), nullptr},
     QString()
    );

  g1.add_lambda_carrier(
    {kcm.get_kcm_type_by_kauvir_type_object( &type_system->type_object__u32() ), nullptr},
     QString()
    );

  table.init_phaon_function(g1, pss, "expand_sample", 710, &expand_sample);

  g1.clear_all();
 }

 {
  g1.add_lambda_carrier(
    {kcm.get_kcm_type_by_kauvir_type_object( &type_system->type_object__str() ), nullptr},
     QString()
    );

  g1.add_lambda_carrier(
    {kcm.get_kcm_type_by_kauvir_type_object( &type_system->type_object__str() ), nullptr},
     QString()
    );

  g1.add_result_carrier(
    {kcm.get_kcm_type_by_kauvir_type_object( &type_system->type_object__str() ), nullptr},
     QString()
    );

  table.init_phaon_function(g1, pss, "test_s_ss", 600, &test_s_ss);

  g1.clear_all();
 }
}

_KANS(Phaon)

#endif // USING_KPH

