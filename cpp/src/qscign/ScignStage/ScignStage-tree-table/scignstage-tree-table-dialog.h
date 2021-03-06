
//          Copyright Nathaniel Christen 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef SCIGNSTAGE_TREE_TABLE_DIALOG__H
#define SCIGNSTAGE_TREE_TABLE_DIALOG__H

#include <QObject>

#include <QMetaType>

#include <QList>

#include <QPoint>

#include <QDialog>
#include <QTableWidget>

#include <functional>

#include "accessors.h"
#include "qsns.h"

#include "nav-protocols/nav-tree-table-1d-panel.h"

#include "subwindows/series-treewidget.h"

#include <functional>

#include "kans.h"

KANS_CLASS_DECLARE(DSM ,Test_Sample)
KANS_CLASS_DECLARE(DSM ,Test_Series)


USING_KANS(DSM)

class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QTabWidget;
class QTextEdit;
class QFrame;
class QHBoxLayout;
class QVBoxLayout;
class QSlider;
class QPlainTextEdit;
class QBoxLayout;
class QButtonGroup;
class QGroupBox;
class QScrollArea;
class QGridLayout;
class QMediaPlayer;
class QProcess;
class QTcpServer;
class QTreeWidget;
class QTreeWidgetItem;


class XPDF_Bridge;

KANS_CLASS_DECLARE(PhaonLib ,Phaon_Runner)
USING_KANS(PhaonLib)

QSNS_(ScignStage)
_QSNS(ScignStage)
//?namespace QScign { namespace ScignStage {



class ScignStage_Tree_Table_Dialog : public QDialog
{

 Q_OBJECT

 QHBoxLayout* minimize_layout_;

 QDialogButtonBox* button_box_;
 QPushButton* button_ok_;
 QPushButton* button_cancel_;
 QPushButton* button_proceed_;


 QHBoxLayout* middle_layout_;
 QVBoxLayout* main_layout_;

 // //  "Pseudo" Toolbar ...
 QHBoxLayout* top_buttons_layout_;

 QPushButton* launch_config_button_;
 QPushButton* activate_tcp_button_;
 QPushButton* take_screenshot_button_;

 QTabWidget* main_tab_widget_;
 Series_TreeWidget* main_tree_widget_;
 Series_TreeWidget* flow_tree_widget_;
 Series_TreeWidget* temperature_tree_widget_;
 Series_TreeWidget* oxy_tree_widget_;

 NAV_Tree_Table1D_Panel* nav_panel_;

 XPDF_Bridge* xpdf_bridge_;

 Test_Series* series_;

 QTcpServer* tcp_server_;

 QString held_xpdf_msg_;

 Phaon_Runner* phr_;

 std::function<void(Phaon_Runner&)> phr_init_function_;
 std::function<void()> screenshot_function_;
 std::function<void()> launch_config_function_;

 Test_Sample* current_sample_;

 quint64 current_tcp_msecs_;

 void* application_model_;

 bool xpdf_is_ready();
 void check_phr();

 void run_msg(QString msg, QByteArray qba);
 void run_kph(const QByteArray& qba);

 void play_file_at_current_index();

 void open_pdf_file(QString name, int page);

 void check_launch_xpdf(std::function<void()> fn,
   std::function<void()> waitfn);
 void send_xpdf_msg(QString msg);

 QString load_about_file(QString name);
 bool ask_pdf_proceed(QString name);
 bool ask_pdf_proceed(Series_TreeWidget::Sort_Options so, quint8 col);
 void show_non_pdf_message(QString name);

 void highlight(QTreeWidget* qtw, int index, int down = -1,
   int up = -1);
 void unhighlight(QTreeWidget* qtw, int index);

 void highlight(Test_Sample* samp);
 void unhighlight(Test_Sample* samp);

 void highlight_scroll_to_sample(Test_Sample* samp);


 void run_tree_context_menu(QVector<Test_Sample*>* samps,
   Series_TreeWidget::Sort_Options so,
   const QPoint& qp, int col, int row = 0);

 void run_tree_context_menu(QVector<Test_Sample*>* samps,
   Series_TreeWidget::Sort_Options so,
   const QPoint& qp, int page, int col,
   std::function<void(int)> pdf_fn,
   std::function<void(int, QVector<Test_Sample*>& samps, bool)> copyc_fn,
   int row = 0,
   std::function<void(int, QVector<Test_Sample*>& samps)> copyr_fn = nullptr,
   std::function<void(int, QVector<Test_Sample*>& samps)> highlight_fn = nullptr);

public:

 ScignStage_Tree_Table_Dialog(XPDF_Bridge* xpdf_bridge,
   Test_Series* series, QWidget* parent = nullptr);

 ~ScignStage_Tree_Table_Dialog();

 ACCESSORS__SET(std::function<void(Phaon_Runner&)>, phr_init_function)
 ACCESSORS__SET(std::function<void()> ,screenshot_function)
 ACCESSORS__SET(std::function<void()> ,launch_config_function)

 ACCESSORS(Test_Series* ,series)
 ACCESSORS(void* ,application_model)

 // //  Kernel Application Interface
 void test_msgbox(QString msg);
 void expand_sample(int index);

 void emit_highlight();
 void uncheck_graphic(QString code);

 void copy_column_to_clipboard(int col, QVector<Test_Sample*>& samps, bool by_rank);
 void copy_column_to_clipboard(int col, bool by_rank);



Q_SIGNALS:
 void canceled(QDialog*);
 void accepted(QDialog*);
 void take_screenshot_requested();
 void launch_config_requested();
 void sample_highlighted(Test_Sample* samp);

 void external_sample_highlighted(QWidget*, Test_Sample* samp);

 void reemit_graphic_open_requested(quint8, quint8, quint8,bool);
 void reemit_graphic_close_requested(quint8, quint8, quint8);


public Q_SLOTS:

 void browse_to_selected_sample(QWidget*, Test_Sample* samp);

 void handle_xpdf_is_ready();
 void handle_take_screenshot_requested();
 void handle_launch_config_requested();

 void handle_sample_down();
 void handle_sample_up();

 void handle_sample_first();
 void handle_peer_first();

 void handle_peer_down();
 void handle_peer_up();

 void handle_flow_down();
 void handle_flow_up();
 void handle_temperature_down();
 void handle_temperature_up();
 void handle_oxy_down();
 void handle_oxy_up();

 void accept();
 void cancel();

 void activate_tcp_requested();


};

//_QSNS(ScignStage)


#endif  // SCIGNSTAGE_AUDIO_DIALOG__H


