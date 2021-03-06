
//          Copyright Nathaniel Christen 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)


#include "config-dialog.h"

#include "styles.h"

#include <QApplication>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QButtonGroup>
#include <QScrollArea>
#include <QFileDialog>
#include <QTabWidget>
#include <QSplitter>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QGroupBox>

#include <QTreeWidget>

#include <QDirIterator>

#include <QPlainTextEdit>
#include <QTextStream>

#include <QComboBox>

#include <QPainter>
#include <QPushButton>
#include <QLabel>

#include <QMessageBox>
#include <QDebug>

#include <QHeaderView>

#include "add-minimize-frame.h"

Config_Dialog::Config_Dialog(QWidget* parent)
  : QDialog(), //QDialog(parent),
    proceed_callback_(nullptr),
    reset_callback_(nullptr)
{
 button_box_ = new QDialogButtonBox(this);

 button_ok_ = new QPushButton("OK");
 button_proceed_ = new QPushButton("Proceed");
 button_cancel_ = new QPushButton("Cancel");

 button_ok_->setDefault(false);
 button_ok_->setAutoDefault(false);
 button_ok_->setEnabled(false);

 button_proceed_->setDefault(false);
 button_proceed_->setAutoDefault(false);
 button_proceed_->setEnabled(true);

 button_cancel_->setDefault(true);

 button_box_->addButton(button_ok_, QDialogButtonBox::AcceptRole);
 button_box_->addButton(button_proceed_, QDialogButtonBox::ApplyRole);
 button_box_->addButton(button_cancel_, QDialogButtonBox::RejectRole);

 button_ok_->setStyleSheet(basic_button_style_sheet_());
 button_proceed_->setStyleSheet(basic_button_style_sheet_());
 button_cancel_->setStyleSheet(basic_button_style_sheet_());


 connect(button_proceed_, SIGNAL(clicked()), this, SLOT(proceed()));
 connect(button_box_, SIGNAL(accepted()), this, SLOT(accept()));
 connect(button_box_, SIGNAL(rejected()), this, SLOT(cancel()));

 main_layout_ = new QVBoxLayout;

 os_group_box_ = new QGroupBox("Operating System Profile");
 os_layout_ = new QHBoxLayout;
 os_combo_box_ = new QComboBox(this);
 os_combo_box_->addItems({"Linux (Generic)",
   "Uhuntu Linux", "Windows", "Mac"});

 b32_checkbox_ = new QCheckBox("32 Bit");
 b64_checkbox_ = new QCheckBox("64 Bit");
 b3264_ = new QButtonGroup(this);

 b3264_->addButton(b32_checkbox_);
 b3264_->addButton(b64_checkbox_);

 os_layout_->addStretch();
 os_layout_->addWidget(os_combo_box_);
 os_layout_->addStretch();
 os_layout_->addWidget(b32_checkbox_);
 os_layout_->addWidget(b64_checkbox_);
 os_layout_->addStretch();

 os_group_box_->setLayout(os_layout_);
 main_layout_->addWidget(os_group_box_);

 compile_options_group_box_ = new QGroupBox("Compile Options", this);
 roles_group_box_ = new QGroupBox("Select User Role", this);

 compile_options_grid_layout_ = new QGridLayout;
 roles_grid_layout_ = new QGridLayout;

 ss3d_check_box_ = new QCheckBox("Use 3d graphics", this);
 compile_options_grid_layout_->addWidget(ss3d_check_box_, 0, 0);

 kph_check_box_ = new QCheckBox("Use Kauvir/Phaon and TCP (for tests)", this);
 compile_options_grid_layout_->addWidget(kph_check_box_, 0, 1, 1, 2);

 xpdf_check_box_ = new QCheckBox("Use XPDF", this);
 compile_options_grid_layout_->addWidget(xpdf_check_box_, 1, 0, 2, 2, Qt::AlignRight);

 xpdf_qt_libs_check_box_ = new QCheckBox("Qt PNG/FreeType libraries", this);
 compile_options_grid_layout_->addWidget(xpdf_qt_libs_check_box_, 1, 2);

 xpdf_system_libs_check_box_ = new QCheckBox("System PNG/FreeType libraries", this);
 compile_options_grid_layout_->addWidget(xpdf_system_libs_check_box_, 2, 2);

 xpdf_qt_libs_check_box_->setEnabled(false);
 xpdf_system_libs_check_box_->setEnabled(false);

 qs_button_group_ = new QButtonGroup(this);

 qs_button_group_->addButton(xpdf_qt_libs_check_box_);
 qs_button_group_->addButton(xpdf_system_libs_check_box_);

 main_button_group_ = new QButtonGroup(this);

 main_button_group_->addButton(ss3d_check_box_);
 main_button_group_->addButton(kph_check_box_);

 kdmi_check_box_ = new QCheckBox("Build KDMI Components and Console (for data export)", this);
 compile_options_grid_layout_->addWidget(kdmi_check_box_, 3, 0, 1, 3);
 main_button_group_->addButton(kdmi_check_box_);

 roic_check_box_ = new QCheckBox("Build Research Object Information Console", this);
 compile_options_grid_layout_->addWidget(roic_check_box_, 4, 0, 1, 3);

 xx_check_box_ = new QCheckBox("Build External XPDF Application", this);
 compile_options_grid_layout_->addWidget(xx_check_box_, 5, 0, 1, 3);

 main_button_group_->addButton(roic_check_box_);
 main_button_group_->addButton(xx_check_box_);

 main_button_group_->setExclusive(false);

 gen_test_check_box_ = new QCheckBox("Preview (right click \"Administrator\" "
    "to enable/disable)", this);
 gen_test_check_box_->setEnabled(false);

 connect(gen_test_check_box_, &QCheckBox::toggled, [this](bool b)
 {
  check_proceed_possible();
 });

 compile_options_grid_layout_->addWidget(gen_test_check_box_, 6, 0, 1, 3, Qt::AlignRight);

 //main_layout_->addWidget(gen_test_check_box_);

 connect(main_button_group_,
   QOverload<QAbstractButton *, bool>::of(&QButtonGroup::buttonToggled),
   [this](QAbstractButton*, bool)
 {
  check_proceed_possible();
 });

 connect(qs_button_group_,
   QOverload<QAbstractButton *, bool>::of(&QButtonGroup::buttonToggled),
   [this](QAbstractButton*, bool)
 {
  check_proceed_possible();
 });

 connect(xpdf_check_box_, &QCheckBox::toggled, [this](bool b)
 {
  xpdf_qt_libs_check_box_->setEnabled(b);
  xpdf_system_libs_check_box_->setEnabled(b);
  if(b)
  {
   QString ac = get_apply_code();
   if(ac.contains("xs") || ac.contains("xq"))
   {

   }
   else
    xpdf_qt_libs_check_box_->setChecked(true);
  }
  check_proceed_possible();
 });

 reset_button_ = new QPushButton("Reset", this);
 reset_button_->setEnabled(false);

 connect(reset_button_, &QPushButton::clicked,
   [this]()
 {
  if(reset_callback_)
    reset_callback_();
  else
    Q_EMIT(reset_requested());
 });

 reset_button_label_ = new QLabel("(reset files to original state;\n"
   "right-click \"Administrator\" to enable/disable)");
 reset_button_label_->setEnabled(false);

 reset_button_layout_ = new QHBoxLayout;
 reset_button_layout_->addStretch();
 reset_button_layout_->addWidget(reset_button_);
 reset_button_layout_->addWidget(reset_button_label_);

 compile_options_grid_layout_->addLayout(reset_button_layout_, 7, 0, 1, 3);

 compile_options_group_box_->setLayout(compile_options_grid_layout_);
 main_layout_->addWidget(compile_options_group_box_);

 roles_button_group_ = new QButtonGroup(this);

 QStringList roles {{"User, Reader, Researcher", "Author", "Editor", "Tester", "Administrator"}};

 int i = 0; int j = 0;
 for(QString r : roles)
 {
  int span = 1;
  if( (i == 0) && (j == 0) )
  {
   span = 2;
   r += " (Default)";
  }
  QCheckBox* ckb = new QCheckBox(r, this);

  connect(ckb, &QCheckBox::toggled, [this](bool b)
  {
   check_proceed_possible();
  });

  if(r == "Administrator")
  {
   ckb->setContextMenuPolicy(Qt::CustomContextMenu);

   connect(ckb, &QCheckBox::customContextMenuRequested, [this](const QPoint& p)
   {
    bool b = !gen_test_check_box_->isEnabled();
    reset_button_->setEnabled(b);
    reset_button_label_->setEnabled(b);
    gen_test_check_box_->setEnabled(b);
   });
  }

  roles_button_group_->addButton(ckb);
  roles_grid_layout_->addWidget(ckb, j, i, 1, span);
  if(i == 2)
  {
   i = 0;
   ++j;
  }
  else
    i += span;
 }

 roles_group_box_->setLayout(roles_grid_layout_);

 main_layout_->addWidget(roles_group_box_);

 autofill_button_ = new QPushButton(
   "Click To Set Compiler Options Based On User Role", this);

 autofill_button_->setStyleSheet(colorful_button_style_sheet_());

 connect(autofill_button_, &QPushButton::clicked, [this]()
 {
  QString rc = get_role_code();
  switch(rc[1].toLatin1())
  {
  case '1': autofill_1(); break;
  case '2': autofill_2(); break;
  case '3': autofill_3(); break;

  case '4': autofill_4(); break;
  case '5': autofill_5(); break;
  case '6': autofill_6(); break;
  }
 });

 QHBoxLayout* autofill_layout_ = new QHBoxLayout;
 autofill_layout_->addStretch();
 autofill_layout_->addWidget(autofill_button_);
 autofill_layout_->addStretch();

 main_layout_->addLayout(autofill_layout_);
 main_layout_->addSpacing(5);
 main_layout_->addStretch();

 minimize_layout_ = add_minimize_frame(button_box_, [this]
 {
#ifdef USE_UBUNTU_MINIMIZE
   this->setWindowFlags(Qt::Window);
   showMinimized();
#else
   setWindowState(Qt::WindowMinimized);
#endif
 });

 main_layout_->addLayout(minimize_layout_);
 //
 //main_layout_->addWidget(button_box_);

 setLayout(main_layout_);
}

void Config_Dialog::autofill_1()
{
 const QSignalBlocker mbl(main_button_group_);
 ss3d_check_box_->setChecked(false);
 kph_check_box_->setChecked(false);
 roic_check_box_->setChecked(false);
 xx_check_box_->setChecked(false);

 if(xpdf_check_box_->isChecked())
 {
  // //  this should trigger the check_proceed_possible()
  xpdf_check_box_->setChecked(false);
 }
 else
   check_proceed_possible();
}

void Config_Dialog::autofill_2(bool ss3d, bool kph, bool xx, bool roic)
{
 const QSignalBlocker mbl(main_button_group_);
 const QSignalBlocker qsbl(qs_button_group_);
 ss3d_check_box_->setChecked(ss3d);
 kph_check_box_->setChecked(kph);
 xpdf_check_box_->setChecked(true);
 xpdf_qt_libs_check_box_->setEnabled(true);
 xpdf_qt_libs_check_box_->setChecked(true);
 xpdf_system_libs_check_box_->setEnabled(true);
 xpdf_system_libs_check_box_->setChecked(false);
 xx_check_box_->setChecked(xx);
 roic_check_box_->setChecked(roic);
 kdmi_check_box_->setChecked(false);
 check_proceed_possible();
}

void Config_Dialog::autofill_3()
{
 autofill_2(true, false, true);
}

void Config_Dialog::autofill_4()
{
 autofill_2(true, false, true, true);
 kdmi_check_box_->setChecked(true);
}

void Config_Dialog::autofill_5()
{
 autofill_2(true, true, true, true);
}

void Config_Dialog::autofill_6()
{
 autofill_2(false, true, false, true);
 kdmi_check_box_->setChecked(true);
}

QString Config_Dialog::get_role_code()
{
 return QString::number( roles_button_group_->checkedId() );
}

QString Config_Dialog::get_apply_code()
{
 QString result;

 if(roic_check_box_->isChecked())
   result += "r";
 if(xx_check_box_->isChecked())
   result += "xx";
 if(xpdf_check_box_->isChecked())
   result += "x";
 if(xpdf_qt_libs_check_box_->isChecked())
   result += "q";
 if(xpdf_system_libs_check_box_->isChecked())
   result += "s";
 if(kph_check_box_->isChecked())
   result += "k";

 if(kdmi_check_box_->isChecked())
   result += "d";

 if(ss3d_check_box_->isChecked())
   result += "3";

 if(gen_test_check_box_->isEnabled() && gen_test_check_box_->isChecked())
   result.prepend("gen_test__");

 return result;
}

Config_Dialog::~Config_Dialog()
{

}


void Config_Dialog::check_proceed_possible()
{
 if(get_apply_code() + get_role_code() == last_ac_)
 {
  button_ok_->setEnabled(true);
  button_cancel_->setEnabled(false);
  button_proceed_->setEnabled(false);
 }
 else
 {
  button_ok_->setEnabled(false);
  button_cancel_->setEnabled(true);
  button_proceed_->setEnabled(true);
  button_proceed_->setDefault(true);
 }
}

void Config_Dialog::register_proceed_completed(QString ac)
{
 last_ac_ = ac;
 button_ok_->setEnabled(true);
 button_cancel_->setEnabled(false);
 button_proceed_->setEnabled(false);
}

void Config_Dialog::proceed()
{
 if(proceed_callback_)
   proceed_callback_(get_apply_code() + get_role_code());
 else
   Q_EMIT(proceed_requested(get_apply_code() + get_role_code()));
}

void Config_Dialog::cancel()
{
 Q_EMIT(rejected());
 Q_EMIT(canceled(this));
 close();
}

void Config_Dialog::accept()
{
 Q_EMIT(accepted(this));
 close();
}
