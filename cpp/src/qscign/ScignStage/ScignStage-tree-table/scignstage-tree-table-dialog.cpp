
//          Copyright Nathaniel Christen 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)


#include "scignstage-tree-table-dialog.h"

#include "defines.h"


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

#include <QtMultimedia/QMediaPlayer>

#include <QPainter>
#include <QPushButton>
#include <QLabel>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QTableWidget>
#include <QGraphicsPixmapItem>

#include <QMessageBox>
#include <QDebug>
#include <QClipboard>

#include <QProcess>

#include <QGraphicsView>
#include <QScrollArea>
#include <QTcpServer>
#include <QNetworkAccessManager>

#include <QHeaderView>

#include <QMenu>
#include <QAction>

#include <QListWidget>

#include "dsmain/test-sample.h"
#include "dsmain/test-series.h"

#include "xpdf-bridge.h"


#include "PhaonLib/phaon-runner.h"

#include "kauvir-phaon/kph-command-package.h"

#include "kauvir-code-model/kcm-channel-group.h"
#include "kauvir-code-model/kauvir-code-model.h"

#include "subwindows/series-treewidget.h"


#include "add-minimize-frame.h"



#include "textio.h"

USING_KANS(TextIO)

USING_KANS(Phaon)

//USING_QSNS(ScignStage)

ScignStage_Tree_Table_Dialog::ScignStage_Tree_Table_Dialog(XPDF_Bridge* xpdf_bridge,
  Test_Series* series, QWidget* parent)
  : QDialog(parent), xpdf_bridge_(xpdf_bridge),
    series_(series), tcp_server_(nullptr),
    phr_(nullptr), phr_init_function_(nullptr),
    screenshot_function_(nullptr),
    launch_config_function_(nullptr),
    current_sample_(nullptr),
    current_tcp_msecs_(0), application_model_(nullptr)
{

 button_box_ = new QDialogButtonBox(this);

 button_ok_ = new QPushButton("OK");
 button_proceed_ = new QPushButton("Proceed");
 button_cancel_ = new QPushButton("Cancel");

 button_ok_->setDefault(false);
 button_ok_->setAutoDefault(false);

 button_proceed_->setDefault(false);
 button_proceed_->setAutoDefault(false);

 button_cancel_->setDefault(true);

 button_ok_->setEnabled(false);

 // // unless this is being embedded ...
 button_proceed_->setEnabled(false);
 button_cancel_->setText("Close");

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


 top_buttons_layout_ = new QHBoxLayout;

 take_screenshot_button_ = new QPushButton("Screenshot", this);

 activate_tcp_button_ = new QPushButton("Activate TCP", this);

 launch_config_button_ = new QPushButton("Customize Build", this);

 take_screenshot_button_->setStyleSheet(colorful_button_style_sheet_());
 activate_tcp_button_->setStyleSheet(colorful_button_style_sheet_());
 launch_config_button_->setStyleSheet(colorful_button_style_sheet_());

 connect(launch_config_button_, SIGNAL(clicked()),
   this, SLOT(handle_launch_config_requested()));

 connect(take_screenshot_button_, SIGNAL(clicked()),
   this, SLOT(handle_take_screenshot_requested()));

 connect(activate_tcp_button_, SIGNAL(clicked()),
   this, SLOT(activate_tcp_requested()));

 top_buttons_layout_->addStretch();

 top_buttons_layout_->addWidget(launch_config_button_);
 top_buttons_layout_->addWidget(activate_tcp_button_);
 top_buttons_layout_->addWidget(take_screenshot_button_);

 main_layout_->addLayout(top_buttons_layout_);

 middle_layout_ = new QHBoxLayout;

 // //   Foreground

 main_tab_widget_ = new QTabWidget(this);

 main_tree_widget_ = new Series_TreeWidget(series_,
   Series_TreeWidget::Sort_Options::Index, this);

 flow_tree_widget_ = new Series_TreeWidget(series_,
   Series_TreeWidget::Sort_Options::Flow, this);

 temperature_tree_widget_ = new Series_TreeWidget(series_,
   Series_TreeWidget::Sort_Options::Temperature, this);

 oxy_tree_widget_ = new Series_TreeWidget(series_,
   Series_TreeWidget::Sort_Options::Oxy, this);

 for(Series_TreeWidget* stw :
   {main_tree_widget_, flow_tree_widget_, temperature_tree_widget_, oxy_tree_widget_})
 {
  connect(stw, &Series_TreeWidget::column_context_menu_requested,
    [this, stw](const QPoint& qp, int col)
  {
   run_tree_context_menu(stw->samples(), stw->sorted_by(), qp, col);
  });

  connect(stw, &Series_TreeWidget::row_context_menu_requested,
    [this, stw](const QPoint& qp, int row, int col)
  {
   run_tree_context_menu(stw->samples(), stw->sorted_by(), qp, col, row);
  });
 }

 main_tab_widget_->addTab(main_tree_widget_, "Main");
 main_tab_widget_->addTab(flow_tree_widget_, "Flow");
 main_tab_widget_->addTab(temperature_tree_widget_, "Temperature");
 main_tab_widget_->addTab(oxy_tree_widget_, "Oxygen");

 main_tab_widget_->setStyleSheet(tab_style_sheet_());

 middle_layout_->addWidget(main_tab_widget_);

 main_layout_->addLayout(middle_layout_);

 nav_panel_ = new NAV_Tree_Table1D_Panel(this);

 connect(nav_panel_, SIGNAL(sample_up_requested()),
   this, SLOT(handle_sample_up()));


 connect(nav_panel_, SIGNAL(graphic_open_requested(quint8, quint8, quint8, bool)),
   this, SIGNAL(reemit_graphic_open_requested(quint8, quint8, quint8, bool)));

 connect(nav_panel_, SIGNAL(graphic_close_requested(quint8, quint8, quint8)),
   this, SIGNAL(reemit_graphic_close_requested(quint8, quint8, quint8)));

 connect(nav_panel_, SIGNAL(sample_down_requested()),
   this, SLOT(handle_sample_down()));

 connect(nav_panel_, SIGNAL(sample_first_requested()),
   this, SLOT(handle_sample_first()));

 connect(nav_panel_, SIGNAL(peer_first_requested()),
   this, SLOT(handle_peer_first()));

 connect(nav_panel_, SIGNAL(peer_up_requested()),
   this, SLOT(handle_peer_up()));

 connect(nav_panel_, SIGNAL(peer_down_requested()),
   this, SLOT(handle_peer_down()));

 main_layout_->addWidget(nav_panel_);

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
 //main_layout_->addWidget(button_box_);

 setLayout(main_layout_);

#ifdef USING_XPDF
 // // xpdf connections ...
 if(xpdf_bridge_)
 {
  connect(xpdf_bridge_, SIGNAL(xpdf_is_ready()),
    this, SLOT(handle_xpdf_is_ready()));
 }
#endif // USING_XPDF

}

QString ScignStage_Tree_Table_Dialog::load_about_file(QString name)
{
 return load_file(QString("%1/%2.txt")
   .arg(ABOUT_FILE_FOLDER).arg(name)).replace('\n', ' ').simplified();
}

bool ScignStage_Tree_Table_Dialog::ask_pdf_proceed(Series_TreeWidget::Sort_Options so,
  quint8 col)
{
 static QMap<QPair<Series_TreeWidget::Sort_Options, quint8>, QString> static_map {{
   {{Series_TreeWidget::Sort_Options::Index, 0}, "!index"},
   {{Series_TreeWidget::Sort_Options::Index, 1}, "?flow"},
   {{Series_TreeWidget::Sort_Options::Index, 2}, "?time-w"},
   {{Series_TreeWidget::Sort_Options::Index, 3}, "?time-a"},
   {{Series_TreeWidget::Sort_Options::Index, 4}, "?temperature"},
   {{Series_TreeWidget::Sort_Options::Index, 5}, "?oxy"},

   {{Series_TreeWidget::Sort_Options::Flow, 0}, "?flow-index"},
   {{Series_TreeWidget::Sort_Options::Flow, 1}, "?flow"},
   {{Series_TreeWidget::Sort_Options::Flow, 2}, "?time-w"},
   {{Series_TreeWidget::Sort_Options::Flow, 3}, "?time-a"},
   {{Series_TreeWidget::Sort_Options::Flow, 4}, "?temperature"},
   {{Series_TreeWidget::Sort_Options::Flow, 5}, "?oxy"},

   {{Series_TreeWidget::Sort_Options::Temperature, 0}, "?temperature-index"},
   {{Series_TreeWidget::Sort_Options::Temperature, 1}, "?flow"},
   {{Series_TreeWidget::Sort_Options::Temperature, 2}, "?time-w"},
   {{Series_TreeWidget::Sort_Options::Temperature, 3}, "?time-a"},
   {{Series_TreeWidget::Sort_Options::Temperature, 4}, "?temperature"},
   {{Series_TreeWidget::Sort_Options::Temperature, 5}, "?oxy"},

   {{Series_TreeWidget::Sort_Options::Oxy, 0}, "?oxy-index"},
   {{Series_TreeWidget::Sort_Options::Oxy, 1}, "?flow"},
   {{Series_TreeWidget::Sort_Options::Oxy, 2}, "?time-w"},
   {{Series_TreeWidget::Sort_Options::Oxy, 3}, "?time-a"},
   {{Series_TreeWidget::Sort_Options::Oxy, 4}, "?temperature"},
   {{Series_TreeWidget::Sort_Options::Oxy, 5}, "?oxy"},
  }};

 QString name = static_map.value({so, col});
 if(name.startsWith('!'))
 {
  show_non_pdf_message(name.mid(1));
  return false;
 }
 if(name.startsWith('?'))
 {
  return ask_pdf_proceed(name.mid(1));
 }
 return false;
}

void ScignStage_Tree_Table_Dialog::show_non_pdf_message(QString name)
{
 QString about = load_about_file(name);
 QMessageBox qmb;
 qmb.setText(about);
 qmb.addButton("Ok", QMessageBox::NoRole);
 qmb.exec();
}


bool ScignStage_Tree_Table_Dialog::ask_pdf_proceed(QString name)
{
 QString about = load_about_file(name);
 QMessageBox qmb;


 int index = about.indexOf("%%");
 if(index == -1)
 {
  qmb.setWindowTitle(name.toUpper());
  qmb.setText(about);
 }
 else
 {
  qmb.setWindowTitle(about.left(index));
  qmb.setText("Click 'Show Details' for a summary "
    "or 'More' for PDF/Original Article links.");
  qmb.setDetailedText(about.mid(index + 2).trimmed());
 }

 qmb.setIcon(QMessageBox::Information);

 QAbstractButton* yes = qmb.addButton(QString("More (PDF) ..."), QMessageBox::YesRole);
 qmb.addButton("Cancel", QMessageBox::NoRole);

 qmb.exec();

 return qmb.clickedButton() == yes;
}

void ScignStage_Tree_Table_Dialog::browse_to_selected_sample(
  QWidget* qw, Test_Sample* samp)
{
 if(current_sample_)
   unhighlight(current_sample_);
 highlight_scroll_to_sample(samp);
 current_sample_ = samp;
 Q_EMIT( external_sample_highlighted(qw, samp) );
}

void ScignStage_Tree_Table_Dialog::highlight_scroll_to_sample(Test_Sample* samp)
{
 highlight(samp);
}


void ScignStage_Tree_Table_Dialog::highlight(Test_Sample* samp)
{
 highlight(main_tree_widget_, samp->index() - 1, 4, 2);
 highlight(flow_tree_widget_, series_->get_flow_rank(*samp) - 1, 4, 2);
 highlight(temperature_tree_widget_, series_->get_temperature_rank(*samp) - 1, 4, 2);
 highlight(oxy_tree_widget_, series_->get_oxy_rank(*samp) - 1, 4, 2);

}

void ScignStage_Tree_Table_Dialog::unhighlight(Test_Sample* samp)
{
 unhighlight(main_tree_widget_, samp->index() - 1);
 unhighlight(flow_tree_widget_, series_->get_flow_rank(*samp) - 1);
 unhighlight(temperature_tree_widget_, series_->get_temperature_rank(*samp) - 1);
 unhighlight(oxy_tree_widget_, series_->get_oxy_rank(*samp) - 1);
}

void ScignStage_Tree_Table_Dialog::highlight(QTreeWidget* qtw, int index,
  int down, int up)
{
 QTreeWidgetItem* twi = qtw->topLevelItem(index);
 twi->setExpanded(true);
 for(int i = 0; i < 6; ++i)
   twi->setForeground(i, QBrush("darkRed"));

 if(Series_TreeWidget* w = qobject_cast<Series_TreeWidget*>(qtw))
 {
  w->highlight_3rd_line(index);
 }

 if(down != -1)
 {
  int max = qMin(index + 4, series_->samples().size() - 1);
  QTreeWidgetItem* mtwi = qtw->topLevelItem(max);
  qtw->scrollToItem(mtwi);
  qtw->scrollToItem(twi);
 }

 if(up != -1)
 {
  int max = qMax(index - up, 0);
  QTreeWidgetItem* mtwi = qtw->topLevelItem(max);
  qtw->scrollToItem(mtwi);

 }
}

void ScignStage_Tree_Table_Dialog::send_xpdf_msg(QString msg)
{
 if(xpdf_bridge_)
   xpdf_bridge_->take_message(msg);
}

void ScignStage_Tree_Table_Dialog::open_pdf_file(QString name, int page)
{
#ifdef USING_XPDF
 check_launch_xpdf([this, name, page]()
 {
  send_xpdf_msg(QString("open:%1;%2").arg(name).arg(page));
 },[this, name, page]()
 {
  held_xpdf_msg_ = QString("open:%1;%2").arg(name).arg(page);
 });
#else
 QMessageBox::warning(this, "XPDF Needed",
   "You need to build the customized XPDF library "
   "to view PDF files from this application.  See "
   "build-order.txt for more information."
 );
#endif
}

void ScignStage_Tree_Table_Dialog::uncheck_graphic(QString code)
{
 nav_panel_->uncheck_graphic(code);
}

void ScignStage_Tree_Table_Dialog::run_tree_context_menu(
  QVector<Test_Sample*>* samps,
  Series_TreeWidget::Sort_Options so, const QPoint& qp,
  int col, int row)
{
 static QVector<quint8> pages {0, 2, 16, 16, 7, 10};

 run_tree_context_menu(samps, so, qp, pages.value(col), col,
 [this, so, col](int page)
 {
  bool proceed = ask_pdf_proceed(so, col);
  if(proceed)
  {
   open_pdf_file(ABOUT_FILE_FOLDER "/main.pdf", page);
  }
 },
 [this](int col, QVector<Test_Sample*>& samps, bool by_rank)
 {
  copy_column_to_clipboard(col, samps, by_rank);
 }, row,
 row? [this](int row, QVector<Test_Sample*>& samps)
 {
  Test_Sample* samp = samps.at(row);
  QString qs = QString("%1 %2 %3 %4 %5 %6")
    .arg(samp->index())
    .arg(samp->flow().getDouble())
    .arg(samp->time_with_flow().getDouble())
    .arg(samp->time_against_flow().getDouble())
    .arg(samp->temperature_adj()/100)
    .arg(samp->oxy());
  QApplication::clipboard()->setText(qs);
 }:(std::function<void(int, QVector<Test_Sample*>& samps)>)nullptr,
 row? [this](int row, QVector<Test_Sample*>& samps)
 {
  if(current_sample_)
  {
   unhighlight(current_sample_);
  }
  current_sample_ = samps.at(row);
  highlight(current_sample_);
 }:(std::function<void(int, QVector<Test_Sample*>& samps)>)nullptr
 );
}

void ScignStage_Tree_Table_Dialog::run_tree_context_menu(
  QVector<Test_Sample*>* samps,
  Series_TreeWidget::Sort_Options so,
  const QPoint& qp,
  int page, int col,
  std::function<void(int)> pdf_fn,
  std::function<void(int, QVector<Test_Sample*>&, bool)> copyc_fn,
  int row,
  std::function<void(int, QVector<Test_Sample*>& samps)> copyr_fn,
  std::function<void(int, QVector<Test_Sample*>& samps)> highlight_fn)
{
 QMenu* qm = new QMenu(this);
 qm->addAction("About/ Show in Document (may require XPDF)",
   [pdf_fn, page](){pdf_fn(page);});

 if(!(
     (so == Series_TreeWidget::Sort_Options::Index) &&
     (col == 0) ))
   qm->addAction("Copy Column to Clipboard (values)",
     [copyc_fn, col, samps](){copyc_fn(col, *samps, false);});

 static QSet<QPair<quint8, quint8>> ranks_ok {

  {(quint8)Series_TreeWidget::Sort_Options::Index, 1},
  {(quint8)Series_TreeWidget::Sort_Options::Index, 4},
  {(quint8)Series_TreeWidget::Sort_Options::Index, 5},

  {(quint8)Series_TreeWidget::Sort_Options::Flow, 0},
  {(quint8)Series_TreeWidget::Sort_Options::Flow, 4},
  {(quint8)Series_TreeWidget::Sort_Options::Flow, 5},
  {(quint8)Series_TreeWidget::Sort_Options::Temperature, 0},
  {(quint8)Series_TreeWidget::Sort_Options::Temperature, 1},
  {(quint8)Series_TreeWidget::Sort_Options::Temperature, 5},
  {(quint8)Series_TreeWidget::Sort_Options::Oxy, 0},
  {(quint8)Series_TreeWidget::Sort_Options::Oxy, 1},
  {(quint8)Series_TreeWidget::Sort_Options::Oxy, 4}
 };

 if(ranks_ok.contains({(quint8)so, col}))
   qm->addAction("Copy Column to Clipboard (ranks)",
     [copyc_fn, col, samps](){copyc_fn(col, *samps, true);});

 if(row)
 {
  if(copyr_fn)
    qm->addAction("Copy Row to Clipboard",
    [copyr_fn, row, samps](){copyr_fn(row, *samps);});

  if(highlight_fn)
    qm->addAction("Highlight (scroll from here)",
    [highlight_fn, row, samps](){highlight_fn(row, *samps);});
 }

 QPoint g = main_tree_widget_->mapToGlobal(qp);
 qm->popup(g);
}



void ScignStage_Tree_Table_Dialog::handle_flow_down()
{
 if(current_sample_)
 {
  int index = series_->get_flow_rank(*current_sample_) - 1;
  unhighlight(current_sample_);
  ++index;
  if(index == series_->samples().size())
  {
   index = 0;
   current_sample_ = flow_tree_widget_->samples()->first();
  }
  else
  {
   current_sample_ = flow_tree_widget_->samples()->at(index);
  }
 }
 else
 {
  current_sample_ = flow_tree_widget_->samples()->first();
 }
 emit_highlight();
 highlight(current_sample_);
}

void ScignStage_Tree_Table_Dialog::handle_flow_up()
{
 if(current_sample_)
 {
  int index = series_->get_flow_rank(*current_sample_) - 1;
  unhighlight(current_sample_);
  if(index == 0)
  {
   index = series_->samples().size() - 1;
   current_sample_ = flow_tree_widget_->samples()->last();
  }
  else
  {
   --index;
   current_sample_ = flow_tree_widget_->samples()->at(index);
  }
 }
 else
 {
  current_sample_ = flow_tree_widget_->samples()->last();
 }
 emit_highlight();
 highlight(current_sample_);
}

void ScignStage_Tree_Table_Dialog::handle_temperature_down()
{
 if(current_sample_)
 {
  int index = series_->get_temperature_rank(*current_sample_) - 1;
  unhighlight(current_sample_);
  ++index;
  if(index == series_->samples().size())
  {
   index = 0;
   current_sample_ = temperature_tree_widget_->samples()->first();
  }
  else
  {
   current_sample_ = temperature_tree_widget_->samples()->at(index);
  }
 }
 else
 {
  current_sample_ = temperature_tree_widget_->samples()->first();
 }
 emit_highlight();
 highlight(current_sample_);
}

void ScignStage_Tree_Table_Dialog::copy_column_to_clipboard(int col, QVector<Test_Sample*>& samps, bool by_rank)
{
 QString copy;
 for(Test_Sample* samp : samps)
 {
  switch (col)
  {
  case 0:
   copy += QString("%1\n").arg(samp->index());
   break;
  case 1:
   copy += QString("%1\n").arg(by_rank?
     series_->get_flow_rank(*samp) : samp->flow().getDouble() );
   break;
  case 2:
   copy += QString("%1\n").arg(samp->time_with_flow().getDouble());
   break;
  case 3:
   copy += QString("%1\n").arg(samp->time_against_flow().getDouble());
   break;
  case 4:
   copy += QString("%1\n").arg(by_rank?
     series_->get_temperature_rank(*samp) : samp->temperature_adj() );
   break;
  case 5:
   copy += QString("%1\n").arg(by_rank?
     series_->get_oxy_rank(*samp) : samp->oxy() );
   break;
  default:
   break;
  }
 }
 QApplication::clipboard()->setText(copy);
}

void ScignStage_Tree_Table_Dialog::copy_column_to_clipboard(int col, bool by_rank)
{
 copy_column_to_clipboard(col, series_->samples(), by_rank);
}

void ScignStage_Tree_Table_Dialog::handle_temperature_up()
{
 if(current_sample_)
 {
  int index = series_->get_temperature_rank(*current_sample_) - 1;
  unhighlight(current_sample_);
  if(index == 0)
  {
   index = series_->samples().size() - 1;
   current_sample_ = temperature_tree_widget_->samples()->last();
  }
  else
  {
   --index;
   current_sample_ = temperature_tree_widget_->samples()->at(index);
  }
 }
 else
 {
  current_sample_ = temperature_tree_widget_->samples()->last();
 }
 emit_highlight();
 highlight(current_sample_);
}

void ScignStage_Tree_Table_Dialog::handle_oxy_down()
{
 if(current_sample_)
 {
  int index = series_->get_oxy_rank(*current_sample_) - 1;
  unhighlight(current_sample_);
  ++index;
  if(index == series_->samples().size())
  {
   index = 0;
   current_sample_ = oxy_tree_widget_->samples()->first();
  }
  else
  {
   current_sample_ = oxy_tree_widget_->samples()->at(index);
  }
 }
 else
 {
  current_sample_ = oxy_tree_widget_->samples()->first();
 }
 emit_highlight();
 highlight(current_sample_);
}

void ScignStage_Tree_Table_Dialog::handle_oxy_up()
{
 if(current_sample_)
 {
  int index = series_->get_oxy_rank(*current_sample_) - 1;
  unhighlight(current_sample_);
  if(index == 0)
  {
   index = series_->samples().size() - 1;
   current_sample_ = oxy_tree_widget_->samples()->last();
  }
  else
  {
   --index;
   current_sample_ = oxy_tree_widget_->samples()->at(index);
  }
 }
 else
 {
  current_sample_ = oxy_tree_widget_->samples()->last();
 }
 emit_highlight();
 highlight(current_sample_);
}



void ScignStage_Tree_Table_Dialog::handle_peer_down()
{
 if(main_tab_widget_->currentWidget() == main_tree_widget_)
   handle_sample_down();
 else if(main_tab_widget_->currentWidget() == flow_tree_widget_)
   handle_flow_down();
 else if(main_tab_widget_->currentWidget() == temperature_tree_widget_)
   handle_temperature_down();
 else if(main_tab_widget_->currentWidget() == oxy_tree_widget_)
   handle_oxy_down();
}

void ScignStage_Tree_Table_Dialog::handle_peer_up()
{
 if(main_tab_widget_->currentWidget() == main_tree_widget_)
   handle_sample_up();
 else if(main_tab_widget_->currentWidget() == flow_tree_widget_)
   handle_flow_up();
 else if(main_tab_widget_->currentWidget() == temperature_tree_widget_)
   handle_temperature_up();
 else if(main_tab_widget_->currentWidget() == oxy_tree_widget_)
   handle_oxy_up();
}

void ScignStage_Tree_Table_Dialog::handle_sample_down()
{
 int index;
 if(current_sample_)
 {
  index = current_sample_->index() - 1;

  unhighlight(current_sample_);
  ++index;
  if(index == series_->samples().size())
  {
   index = 0;
   current_sample_ = series_->samples().first();
  }
  else
  {
   current_sample_ = series_->samples().at(index);
  }
 }
 else
 {
  index = 0;
  current_sample_ = series_->samples().first();
 }
 emit_highlight();

 highlight(current_sample_);

}

void ScignStage_Tree_Table_Dialog::emit_highlight()
{
 Q_EMIT(sample_highlighted(current_sample_));
}

void ScignStage_Tree_Table_Dialog::unhighlight(QTreeWidget* qtw, int index)
{
 QTreeWidgetItem* twi = qtw->topLevelItem(index);
 twi->setExpanded(false);
 for(int i = 0; i < 6; ++i)
   twi->setForeground(i, QBrush("black"));

 if(Series_TreeWidget* w = qobject_cast<Series_TreeWidget*>(qtw))
 {
  w->highlight_3rd_line(index);
 }
}

void ScignStage_Tree_Table_Dialog::handle_sample_up()
{
 int index;
 if(current_sample_)
 {
  index = current_sample_->index() - 1;
  unhighlight(current_sample_);
  if(index == 0)
  {
   index = series_->samples().size() - 1;
   current_sample_ = series_->samples().last();
  }
  else
  {
   --index;
   current_sample_ = series_->samples().at(index);
  }
 }
 else
 {
  index = series_->samples().size() - 1;
  current_sample_ = series_->samples().last();
 }
 emit_highlight();
 highlight(current_sample_);
}

void ScignStage_Tree_Table_Dialog::handle_peer_first()
{
 if(current_sample_)
 {
  unhighlight(current_sample_);
 }

 QWidget* cw = main_tab_widget_->currentWidget();
 if(Series_TreeWidget* stw = qobject_cast<Series_TreeWidget*>(cw))
 {
  QVector<Test_Sample*>* samps = stw->samples();
  current_sample_ = samps->first();
  emit_highlight();

  highlight(current_sample_);

 }
}

void ScignStage_Tree_Table_Dialog::expand_sample(int index)
{
 if(current_sample_)
 {
  unhighlight(current_sample_);
 }
 current_sample_ = series_->samples().at(index - 1);
 highlight(current_sample_);
}

void ScignStage_Tree_Table_Dialog::handle_sample_first()
{
 if(current_sample_)
 {
  unhighlight(current_sample_);
 }
 current_sample_ = series_->samples().first();
 emit_highlight();

 QTreeWidgetItem* twi = main_tree_widget_->topLevelItem(0);
 highlight(current_sample_);
 main_tree_widget_->scrollToItem(twi);
}

void ScignStage_Tree_Table_Dialog::handle_launch_config_requested()
{
 Q_EMIT(launch_config_requested());
 if(launch_config_function_)
   launch_config_function_();
}

void ScignStage_Tree_Table_Dialog::handle_take_screenshot_requested()
{
 Q_EMIT(take_screenshot_requested());
 if(screenshot_function_)
   screenshot_function_();
}

void ScignStage_Tree_Table_Dialog::check_phr()
{
#ifdef USING_KPH
 if(!phr_)
 {
  phr_ = new Phaon_Runner;
  if(phr_init_function_)
    phr_init_function_(*phr_);
 }
#endif
}

// // KAI
void ScignStage_Tree_Table_Dialog::test_msgbox(QString msg)
{
 QString m = QString("Received #message: %1").arg(msg);
 QMessageBox::information(this, "Test OK", m);
}



bool ScignStage_Tree_Table_Dialog::xpdf_is_ready()
{
 if(xpdf_bridge_)
   return xpdf_bridge_->is_ready();
 return false;
}

ScignStage_Tree_Table_Dialog::~ScignStage_Tree_Table_Dialog()
{

}

void ScignStage_Tree_Table_Dialog::handle_xpdf_is_ready()
{
 if(!held_xpdf_msg_.isEmpty())
 {
  send_xpdf_msg(held_xpdf_msg_);
  held_xpdf_msg_.clear();
 }
}

void ScignStage_Tree_Table_Dialog::check_launch_xpdf(std::function<void()> fn,
  std::function<void()> waitfn)
{
 if(xpdf_is_ready())
 {
  fn();
  return;
 }

 if(xpdf_bridge_)
 {
  xpdf_bridge_->init();
  waitfn();
  return;
 }
}

void ScignStage_Tree_Table_Dialog::run_kph(const QByteArray& qba)
{
#ifdef USING_KPH
 check_phr();

 KPH_Command_Package khp;
 khp.absorb_data(qba);

 Kauvir_Code_Model& kcm = phr_->get_kcm();

 KCM_Channel_Group kcg(kcm.channel_names());

 khp.init_channel_group(kcm, kcg);
 phr_->run(kcg);
#endif
}


void ScignStage_Tree_Table_Dialog::run_msg(QString msg, QByteArray qba)
{
 qDebug() << QString("received: %1").arg(msg);

 if(msg == "kph")
 {
  run_kph(qba);
 }
}

void ScignStage_Tree_Table_Dialog::activate_tcp_requested()
{
#ifdef USING_KPH
 QString waiting_message;

 if(tcp_server_)
 {
  waiting_message = QString("TCP is already started, using IP: %1\nport: %2\n\n")
    .arg(tcp_server_->serverAddress().toString()).arg(tcp_server_->serverPort());
  QMessageBox::information(this, "Already Activated", waiting_message);
  return;
 }
 tcp_server_ = new QTcpServer();
 QMap<qintptr, QByteArray>* temps = new QMap<qintptr, QByteArray>();

 int port = 18261; // // r z 1

 if (!tcp_server_->listen(QHostAddress::LocalHost, port))
 {
  QMessageBox::critical(this, "TCP Initialization Failed",
                         QString("Could not use port: %1.")
                         .arg(tcp_server_->errorString()));
 }
 else
 {
  waiting_message = QString("Using IP: %1\nport: %2\n\n")
     .arg(tcp_server_->serverAddress().toString()).arg(tcp_server_->serverPort());

  QMessageBox::information(this, "TCP is Started",
                            QString(waiting_message));
 }

 QObject::connect(tcp_server_, &QTcpServer::newConnection, [this, temps]
 {
  QTcpSocket* clientConnection = tcp_server_->nextPendingConnection();
  QObject::connect(clientConnection, &QAbstractSocket::disconnected,
    clientConnection, &QObject::deleteLater);
  clientConnection->write("OK");
  QObject::connect(clientConnection, &QTcpSocket::readyRead, [clientConnection, this, temps]
  {
   qintptr sd = clientConnection->socketDescriptor();
   QByteArray received;
   while(clientConnection->bytesAvailable())
   {
    received.append(clientConnection->readAll());
   }
   if(received.endsWith("<//>"))
   {
    received.chop(4);
    QByteArray qba = (*temps)[sd];
    qba.append(received);
    temps->remove(sd);

    int index = qba.indexOf("<<>>");

    if(index != -1)
    {
     int i1 = qba.indexOf('@', index);
     int i2 = qba.indexOf(':', i1);
     QString msg = QString::fromLatin1(qba.mid(index + 4, i1 - index - 4));
     QByteArray ms = qba.mid(i1 + 1, i2 - i1 - 2);
     quint64 msecs = ms.toLongLong();
     if(msecs != current_tcp_msecs_)
     {
      current_tcp_msecs_ = msecs;
      run_msg( msg, qba.mid(i2 + 1) );
     }
    }
    clientConnection->write("END");
   }
   else
   {
    (*temps)[sd] += received;
   }
  });
 });
#else
 QMessageBox::critical(this, "Kauvir/Phaon Needed",
   QString(
     "To use TCP for testing/demoing \"Kernel Application Interface\" "
     "functions you need to build several additional libraries "
     "(see the build-order.txt file for Qt project files and %1, line %2).")
     .arg(__FILE__).arg(__LINE__)
 );
#endif
}

void ScignStage_Tree_Table_Dialog::play_file_at_current_index()
{

}

void ScignStage_Tree_Table_Dialog::cancel()
{
 Q_EMIT(rejected());
 Q_EMIT(canceled(this));
 Q_EMIT(rejected());
 close();
}

void ScignStage_Tree_Table_Dialog::accept()
{
 Q_EMIT(accepted(this));
}
