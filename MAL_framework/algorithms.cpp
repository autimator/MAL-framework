#include "algorithms.h"
#include "ui_algorithms.h"
#include "widget.h"



#include "comp_algo.h"

Algorithms::Algorithms(Widget *widgetref,QWidget *parent) : //constructor
    QWidget(parent),ui(new Ui::Algorithms)
{
    ui->setupUi(this);
    widget=widgetref;
    //initialise the comp_data variables
    if(widget->comp_data.Signal_algorithms==false)
    {
        if(ui->rb_par_coop->isChecked()==true)widget->comp_data.Par_comp=false;//Par opt
        else widget->comp_data.Par_comp=true;
        widget->comp_data.TFT_smart=ui->cb_smart->isChecked();//TFT
        ui->lbl_egr_expl->setText(QString::number(ui->slid_Egr_expl->value()));//EGreedy
        widget->comp_data.Egr_expl=ui->slid_Egr_expl->value();
        ui->lbl_markov_win->setText(QString::number(ui->slid_Mark_w->value()));//Markov
        widget->comp_data.Mark_win=ui->slid_Mark_w->value();
        ui->lbl_sat_pers->setText(QString::number(ui->slid_Sat_pers->value()));//Satisfysing play
        widget->comp_data.Sat_pers_rate=static_cast<double>(ui->slid_Sat_pers->value())/100;
        widget->comp_data.Sat_aspir=ui->le_sat_aspir->text().toDouble();
        ui->lbl_ql_expl->setText(QString::number(ui->slid_ql_expl->value()));//Qlearning
        widget->comp_data.Ql_expl=static_cast<double>(ui->slid_ql_expl->value())/100;
        ui->lbl_ql_expl_dec->setText(QString::number(ui->slid_expl_dec->value()));//Qlearning
        widget->comp_data.Ql_dec_expl=static_cast<double>(ui->slid_expl_dec->value())/100;
        ui->lbl_ql_learn->setText(QString::number(ui->slid_ql_learn->value()));//Qlearning
        widget->comp_data.Ql_lear=static_cast<double>(ui->slid_ql_learn->value())/100;
        ui->lbl_ql_learn_dec->setText(QString::number(ui->slid_QL_learn_Dec->value()));//Qlearning
        widget->comp_data.Ql_dec_l=static_cast<double>(ui->slid_QL_learn_Dec->value())/100;
        ui->lbl_ql_disc->setText(QString::number(ui->slid_QL_disc->value()));//Qlearning
        widget->comp_data.Ql_disc=static_cast<double>(ui->slid_QL_disc->value())/100;
    }
    else read_compdat();//comp_data filled with intell from session.
}

Algorithms::~Algorithms()//destructor
{
    delete ui;
}




//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//a test button used for various purposes with test code bellow
//-----------------------------------------------------------------------------------------------------

void Algorithms::on_pushButton_clicked()
{
/*
//bully test...
    Game testgame;
    testgame.Matrix_i={{6,3},{1,8},{2,3},{1,8},{3,6},{5,9},{3,7},{4,9},{5,8}};
    testgame.Pareto_opt={{5,4},{8,1}};//,{3,6},{2,8},{6,3}};//{{-1,-1},{0,-4},{-4,0}};//{{5,4},{8,1},{3,6},{2,8},{6,3}};//5,4 8,1 3,6 2,8 6,3
    testgame.Pareto_opt_ind={3,0};//,2,3,4};//{0,1,2};//{0,1,2,3,4};
    testgame.Actions=3;
    testgame.Players=2;
    testgame.Calculate_stepsize();

    qDebug()<<"aa"<<testgame.Matrix_i;
    QVector<Comp_algo*> algos;
    IterParam pars;

    pars.plnum=0;
    pars.game=&testgame;
    algos.append(new Bully(&pars));
*/

  QVector<Comp_algo*> algos;
    IterParam pars;
    pars.game=&widget->G_data[0];
    //pars.Steps=pars.game->Stepsize;
    //for(int k = 0; k < (pars.Steps.size()/2); k++) std::swap(pars.Steps[k],pars.Steps[pars.Steps.size()-(1+k)]); //Steps.swap(k,Steps.size()-(1+k));//reverse stepsize
    //determined, edit NE_d
    /*pars.game->NE_d=QVector<QVector<QVector<double>>>();
    pars.plnum=0;
    algos.append(new Nash(&pars));
    pars.game->NE_d.append({{0,1},{0,1}});
    pars.game->NE_d.append({{0.4,0.6},{0.3,0.7}});
    */
    //double learnrate, double discountrate, double explorerate, double decay_learn, double decay_explore
    pars.plnum=0;
    algos.append(new Markov(&pars,2));
    //algos.append(new Godfather(&pars));
    //pars.plnum=1;
    //algos.append(new Bully());
    pars.past_act={0,0};
    qDebug()<<"init";
    for(int i=0;i<20;i++)
    {
        pars.iteration=i;
        qDebug()<<"------------------";
        pars.plnum=1;
        pars.past_act[0]=algos[0]->action(&pars);
        //pars.plnum=1;
        //pars.past_act[1]=algos[1]->action(&pars);

        //qDebug()<<pars.past_act[0];
        //if(i<2){pars.past_act[1]=0;}//pars.past_rews={1,0};}
        //else {pars.past_act[1]=1;}//pars.past_rews={0,1};}
        int cel=pars.game->Calculate_bincel(pars.past_act,pars.game->Stepsize);
        pars.past_rews=pars.game->Matrix_i[cel];

        qDebug()<<"past_act"<<pars.past_act;
        qDebug()<<"past_rews"<<pars.past_rews;
        //qDebug()<<"cel"<<cel;
    }


    // A temporary array to store all combination one by one
/*    QList<QString> arr = {"1","2","3","4","5","6","7","8","9","10"};
    int num_pl = 2;
//    int n = arr.count();
    QList<QString> data;
    for(int i=0;i<num_pl;i++)data.append("");
    bool symmertric=false;
    // Print all combination using temprary array 'data[]'
    //combinationUtil(arr, data, 0, arr.count()-1, 0, num_pl );
    //all_combination(arr, num_pl,symmertric);
    arr = {"1","2","3","4","5","6","7","8","9"};
    all_combination(arr, num_pl,symmertric);
    arr = {"1","2","3","4","5","6","7","8"};
    all_combination(arr, num_pl,symmertric);
    arr = {"1","2","3","4","5","6","7"};
    all_combination(arr, num_pl,symmertric);
    arr = {"1","2","3","4","5","6"};
    all_combination(arr, num_pl,symmertric);
    arr = {"1","2","3","4","5"};
    all_combination(arr, num_pl,symmertric);

    arr = {"1","2","3","4"};
    all_combination(arr, num_pl,symmertric);

    arr = {"1","2","3"};
    all_combination(arr, num_pl,symmertric);
    arr = {"1","2"};
    all_combination(arr, num_pl,symmertric);
*/


    /*REMEMBER: PLAYER 0 is the column player!!!!!!!!!!!!!!!!!!*/
    //determined test
    /*
    Comp_algo test_algo(&widget->g_data[0]);//init on game 0
    qDebug()<<"marker";
    test_algo.determined_init(&widget->g_data[0],0);//init pl0
    qDebug()<<"move";
    qDebug()<<"result"<<test_algo.determined_action(0);
    */
/*
    //ficticious play test
    test_algo.fict_pl_init(&widget->g_data[0],0);//init pl0
    qDebug()<<"move";
    qDebug()<<"result"<<test_algo.fict_pl_action(&widget->g_data[0],{0,0},0);//first move pl 0: random
    qDebug()<<"result"<<test_algo.fict_pl_action(&widget->g_data[0],{0,1},0);//second move. pl0
    qDebug()<<"result"<<test_algo.fict_pl_action(&widget->g_data[0],{0,0},0);//random:equall
    qDebug()<<"result"<<test_algo.fict_pl_action(&widget->g_data[0],{0,0},0);//act 0
*/
}



/* arr[]  ---> Input Array
   data[] ---> Temporary array to store current combination
   start & end ---> Staring and Ending indexes in arr[]
   index  ---> Current index in data[]
   r ---> num_players */
void Algorithms::combinationUtil(QList<QString> arr, QList<QString> data, int start, int end,int index, int r)
{
    //qDebug()<<"b"<<arr<<data<<start<<end<<index<<r;
    if (index == r)// combination is ready
    {
/*        for(int i=0;i<r;i++)
        {
            qDebug()<<data;
            data.append(data.takeFirst());
        }*/
        qDebug()<<data;
        //qDebug()<<"----";
        return;
    }
    for (int i=start; i<=end && end-i+1 >= r-index; i++)
    {
        /*if(index==0)//selfplay
        {
            for(int j=0;j<r;j++) data[j] = arr[i];
            qDebug()<<data;
        }*/
        //qDebug()<<"aa"<<i<<index;
        data[index] = arr[i];
        //qDebug()<<"a"<<arr<<data<<i+1<<end<<index+1<<r;
        combinationUtil(arr, data, i+1, end, index+1, r);   //---------------------------------------
        //assymetric call itself without incrementing i!!
        //qDebug()<<"c"<<arr<<data<<i<<end<<index<<r;
    }    
}

void Algorithms::all_combination(QList<QString> players, int num_pl,bool symmertric)
{
    int num_algos=players.count();
    int counter=0;
    //symmetric game??
    QList<int> binpl;
    QList<QString> data;
//    num_algos=num_algos-1;//we count from zero
    for(int i=0;i<num_pl;i++){binpl.append(0);data.append(players[0]);}
    bool done=false;

    while(done==false)
    {
        data[0]=players[binpl[0]];
//        qDebug()<<binpl;
        qDebug()<<data;
//        pl.append(data);
//        run_comparison(data);
        counter++;
        binpl[0]++;        
        if(binpl[0]>=num_algos)//update nextnum
        {
            for(int i=0;i<num_pl;i++)
            {
                if(binpl[i]>=num_algos)
                {
//                    if(binpl[i+1]<=num_algos)data[i+1]=players[binpl[i+1]];
//                    data[i]=players[0];
                    if((i+1)<num_pl)
                    {
                        binpl[i+1]++;
                        if(symmertric==false) //asymmetric games
                        {
                            binpl[i]=0;
                            if(binpl[i+1]<num_algos)data[i+1]=players[binpl[i+1]];
                            data[i]=players[0];
                        }
                        else //symmetric games
                        {
                            for(int j=i;j>=0;j--)binpl[j]=binpl[j+1];
                            if(binpl[i+1]<num_algos){data[i+1]=players[binpl[i+1]];data[i]=players[binpl[i]];}
                        }
                    }
                    else{ done=true;}
                }
            }
        }
    }
    qDebug()<<"counter"<<counter;
}





//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//only UI stuff
//functions write directly to comp_data as this class is not called when settings starts the game.
//-----------------------------------------------------------------------------------------------------
//show data from comp data on screen after loading session.
//slot called from settings!!
void Algorithms::read_compdat()
{
    //qDebug()<<"received update signal";
    //step0:avoid double adding algorithms by first removing them...
     QVector<QString> Algotihms=widget->comp_data.Comparing_Algos;
     widget->comp_data.Comparing_Algos= QVector<QString>();
     ui->cb_best_resp->setChecked(false);
     ui->CB_Bully->setChecked(false);
     ui->cb_al_det->setChecked(false);
     ui->cb_Egreedy->setChecked(false);
     ui->cb_al_fict->setChecked(false);
     ui->CB_GodFather->setChecked(false);
     ui->cb_Markov->setChecked(false);
     ui->cb_Nash->setChecked(false);
     ui->cb_Ngreedy->setChecked(false);
     ui->cb_al_par_opt->setChecked(false);
     ui->cb_ql->setChecked(false);
     ui->cb_al_rand->setChecked(false);
     ui->cb_satisfising->setChecked(false);
     ui->cb_TFT->setChecked(false);

    //step one: what algorithms are used
    if(Algotihms.contains("best_resp"))ui->cb_best_resp->setChecked(true);
    else ui->cb_best_resp->setChecked(false);
    if(Algotihms.contains("bully"))ui->CB_Bully->setChecked(true);
    else ui->CB_Bully->setChecked(false);
    if(Algotihms.contains("determined"))ui->cb_al_det->setChecked(true);
    else ui->cb_al_det->setChecked(false);
    if(Algotihms.contains("egreedy"))ui->cb_Egreedy->setChecked(true);
    else ui->cb_Egreedy->setChecked(false);
    if(Algotihms.contains("fict_play"))ui->cb_al_fict->setChecked(true);
    else ui->cb_al_fict->setChecked(false);
    if(Algotihms.contains("godfather"))ui->CB_GodFather->setChecked(true);
    else ui->CB_GodFather->setChecked(false);
    if(Algotihms.contains("markov"))ui->cb_Markov->setChecked(true);
    else ui->cb_Markov->setChecked(false);
    if(Algotihms.contains("maxmin"))ui->cb_Ngreedy->setChecked(true);
    else ui->cb_Ngreedy->setChecked(false);
    if(Algotihms.contains("nash"))ui->cb_Nash->setChecked(true);
    else ui->cb_Nash->setChecked(false);
    if(Algotihms.contains("ngreedy"))ui->cb_Ngreedy->setChecked(true);
    else ui->cb_Ngreedy->setChecked(false);
    if(Algotihms.contains("noreg"))ui->cb_Noregret->setChecked(true);
    else ui->cb_Noregret->setChecked(false);
    if(Algotihms.contains("pareto_opt"))ui->cb_al_par_opt->setChecked(true);
    else ui->cb_al_par_opt->setChecked(false);
    if(Algotihms.contains("ql"))ui->cb_ql->setChecked(true);
    else ui->cb_ql->setChecked(false);
    if(Algotihms.contains("random"))ui->cb_al_rand->setChecked(true);
    else ui->cb_al_rand->setChecked(false);
    if(Algotihms.contains("satisF"))ui->cb_satisfising->setChecked(true);
    else ui->cb_satisfising->setChecked(false);
    if(Algotihms.contains("TFT"))ui->cb_TFT->setChecked(true);
    else ui->cb_TFT->setChecked(false);

    //set two, additional settings
    ui->lbl_egr_expl->setText(QString::number(widget->comp_data.Egr_expl));//Egr_expl
    ui->slid_Egr_expl->setValue(widget->comp_data.Egr_expl);
    ui->lbl_markov_win->setText(QString::number(widget->comp_data.Mark_win));//markov window
    ui->slid_Mark_w->setValue(widget->comp_data.Mark_win);
    if(widget->comp_data.Par_comp==true){ui->rb_par_coop->setChecked(false);ui->rb_par_comp->setChecked(true);}//Par_comp
    else{ui->rb_par_coop->setChecked(true);ui->rb_par_comp->setChecked(false);}
    ui->lbl_ql_expl->setText(QString::number(static_cast<int>(widget->comp_data.Ql_expl*100)));//QL
    ui->slid_ql_expl->setValue(static_cast<int>(widget->comp_data.Ql_expl*100));
    ui->lbl_ql_expl_dec->setText(QString::number(static_cast<int>(widget->comp_data.Ql_dec_expl*100)));
    ui->slid_expl_dec->setValue(static_cast<int>(widget->comp_data.Ql_dec_expl*100));
    ui->lbl_ql_learn->setText(QString::number(static_cast<int>(widget->comp_data.Ql_lear*100)));
    ui->slid_ql_learn->setValue(static_cast<int>(widget->comp_data.Ql_lear*100));
    ui->lbl_ql_learn_dec->setText(QString::number(static_cast<int>(widget->comp_data.Ql_dec_l*100)));
    ui->slid_QL_learn_Dec->setValue(static_cast<int>(widget->comp_data.Ql_dec_l*100));
    ui->lbl_ql_disc->setText(QString::number(static_cast<int>(widget->comp_data.Ql_disc*100)));
    ui->slid_QL_disc->setValue(static_cast<int>(widget->comp_data.Ql_disc*100));

    ui->lbl_sat_pers->setText(QString::number(static_cast<int>(widget->comp_data.Sat_pers_rate*100)));//satifysing
    ui->slid_Sat_pers->setValue(static_cast<int>(widget->comp_data.Sat_pers_rate*100));
    ui->le_sat_aspir->setText(QString::number(widget->comp_data.Sat_aspir));
    if(widget->comp_data.TFT_smart==true)ui->cb_smart->setChecked(true);//TFT
    else ui->cb_smart->setChecked(false);

}

void Algorithms::on_cb_best_resp_toggled(bool checked)
{
    if(checked==true)widget->comp_data.Comparing_Algos.append("best_resp");
    else  widget->comp_data.Comparing_Algos.removeOne("best_resp");
}

void Algorithms::on_CB_Bully_toggled(bool checked)
{
    if(checked==true)widget->comp_data.Comparing_Algos.append("bully");
    else  widget->comp_data.Comparing_Algos.removeOne("bully");
}

void Algorithms::on_cb_al_det_toggled(bool checked)
{
    if(checked==true)widget->comp_data.Comparing_Algos.append("determined");
    else  widget->comp_data.Comparing_Algos.removeOne("determined");
}

void Algorithms::on_cb_Egreedy_toggled(bool checked)
{
    if(checked==true)widget->comp_data.Comparing_Algos.append("egreedy");
    else  widget->comp_data.Comparing_Algos.removeOne("egreedy");
}
void Algorithms::on_slid_Egr_expl_valueChanged(int value)
{
    ui->lbl_egr_expl->setText(QString::number(value));
    widget->comp_data.Egr_expl=value;
}
void Algorithms::on_cb_al_fict_toggled(bool checked)
{
    if(checked==true)widget->comp_data.Comparing_Algos.append("fict_play");
    else  widget->comp_data.Comparing_Algos.removeOne("fict_play");
}

void Algorithms::on_CB_GodFather_toggled(bool checked)
{
    if(checked==true)widget->comp_data.Comparing_Algos.append("godfather");
    else  widget->comp_data.Comparing_Algos.removeOne("godfather");
}

void Algorithms::on_cb_Markov_toggled(bool checked)//markov
{
    //cb_Markov
    if(checked==true)widget->comp_data.Comparing_Algos.append("markov");
    else  widget->comp_data.Comparing_Algos.removeOne("markov");
}

void Algorithms::on_slid_Mark_w_valueChanged(int value)
{
    ui->lbl_markov_win->setText(QString::number(value));
    widget->comp_data.Mark_win=value;
}

void Algorithms::on_CB_Maxmin_toggled(bool checked)
{
    if(checked==true)widget->comp_data.Comparing_Algos.append("maxmin");
    else  widget->comp_data.Comparing_Algos.removeOne("maxmin");
}

void Algorithms::on_cb_Nash_toggled(bool checked)
{
    if(checked==true)widget->comp_data.Comparing_Algos.append("nash");
    else  widget->comp_data.Comparing_Algos.removeOne("nash");
}

void Algorithms::on_cb_Ngreedy_toggled(bool checked)
{
    if(checked==true)widget->comp_data.Comparing_Algos.append("ngreedy");
    else  widget->comp_data.Comparing_Algos.removeOne("ngreedy");
}

void Algorithms::on_cb_Noregret_toggled(bool checked)
{
    if(checked==true)widget->comp_data.Comparing_Algos.append("noreg");
    else  widget->comp_data.Comparing_Algos.removeOne("noreg");
}

void Algorithms::on_cb_al_par_opt_toggled(bool checked)
{
    if(checked==true)widget->comp_data.Comparing_Algos.append("pareto_opt");
    else  widget->comp_data.Comparing_Algos.removeOne("pareto_opt");
}

void Algorithms::on_rb_par_comp_toggled(bool checked)//rb_par_comp
{
    if(checked==true){ui->rb_par_coop->setChecked(false);widget->comp_data.Par_comp=true;}
}
void Algorithms::on_rb_par_coop_toggled(bool checked)
{
    if(checked==true){ui->rb_par_comp->setChecked(false);widget->comp_data.Par_comp=false;}
}

void Algorithms::on_cb_ql_toggled(bool checked)
{
    if(checked==true)widget->comp_data.Comparing_Algos.append("ql");
    else  widget->comp_data.Comparing_Algos.removeOne("ql");
}

void Algorithms::on_cb_al_rand_toggled(bool checked)
{
    if(checked==true)widget->comp_data.Comparing_Algos.append("random");
    else  widget->comp_data.Comparing_Algos.removeOne("random");
}

void Algorithms::on_cb_satisfising_toggled(bool checked)
{
    if(checked==true)widget->comp_data.Comparing_Algos.append("satisF");
    else  widget->comp_data.Comparing_Algos.removeOne("satisF");
}

void Algorithms::on_slid_Sat_pers_valueChanged(int value)
{
    ui->lbl_sat_pers->setText(QString::number(value));
    widget->comp_data.Sat_pers_rate=static_cast<double>(value)/100;
}

void Algorithms::on_le_sat_aspir_editingFinished()
{
    widget->comp_data.Sat_aspir=ui->le_sat_aspir->text().toDouble();
}

void Algorithms::on_cb_TFT_toggled(bool checked)
{
    if(checked==true)widget->comp_data.Comparing_Algos.append("TFT");
    else  widget->comp_data.Comparing_Algos.removeOne("TFT");
}

void Algorithms::on_cb_smart_toggled(bool checked)
{
    widget->comp_data.TFT_smart=checked;
}







void Algorithms::on_slid_ql_expl_valueChanged(int value)//ql
{
    ui->lbl_ql_expl->setText(QString::number(value));
    widget->comp_data.Ql_expl=static_cast<double>(value)/100;
}
void Algorithms::on_slid_expl_dec_valueChanged(int value)
{
    ui->lbl_ql_expl_dec->setText(QString::number(value));
    widget->comp_data.Ql_dec_expl=static_cast<double>(value)/100;
}
void Algorithms::on_slid_ql_learn_valueChanged(int value)
{
    ui->lbl_ql_learn->setText(QString::number(value));
    widget->comp_data.Ql_lear=static_cast<double>(value)/100;

}
void Algorithms::on_slid_QL_learn_Dec_valueChanged(int value)
{
    ui->lbl_ql_learn_dec->setText(QString::number(value));
    widget->comp_data.Ql_dec_l=static_cast<double>(value)/100;

}
void Algorithms::on_slid_QL_disc_valueChanged(int value)
{
    ui->lbl_ql_disc->setText(QString::number(value));
    widget->comp_data.Ql_disc=static_cast<double>(value)/100;
}




void Algorithms::on_Pb_tune_clicked()
{
    int num_fcalls=50;//number of function calls
    dlib::function_spec spec_F({2}, {50},{true});//true: ints
    //dlib::function_spec spec_F({0,0}, {100,1},{true,false});//satisfysing
    //dlib::function_spec spec_F({0,0,0,0,0}, {1,1,1,1,1},{false,false,false,false,false});//paramater range, vectors: for each param, 'true' signals its an integer..
    dlib::global_function_search opt(spec_F);
/*
    //the optimisation loop
    for (int i = 0; i < num_fcalls; ++i)
    {
        dlib::function_evaluation_request next = opt.get_next_x();

        //double a =static_cast<int>(next.x()(0));
        double a = next.x()(0);
        //qDebug()<<a;
        //double b = next.x()(1);
        //double c = next.x()(2);
        //double d = next.x()(3);
        //double e = next.x()(4);
        //double a=0.9;
        double b=0.9;
        double c=0.2;
        double d=0.999913;
        double e=0.999941;
        qDebug()<<"a,b,c,d,e"<<a;//<<b<<c<<d<<e;
        double res=EGr_tune(a,b,c,d,e);
        next.set(res);  // Tell the solver what happened.
    }

    // Find out what point gave the largest outputs:
    dlib::matrix<double,0,1> x;
    double y;
    size_t function_idx;
    opt.get_best_function_eval(x,y,function_idx);

    qDebug() << "function_idx: "<< function_idx;
    qDebug() << "y: " << y;
    qDebug()<<"x0:"<< x(0,0); //x0
    qDebug()<<"x1:"<< x(0,1); //x1
    qDebug()<<"x2:"<< x(0,2); //x2
    qDebug()<<"x3:"<< x(0,3); //x1
    qDebug()<<"x4:"<< x(0,4); //x1
*/

    //bully tune
    double a=0,b=0,c=0,d=0,e=0;
    double res=EGr_tune(a,b,c,d,e);
    qDebug()<<"bully earned"<<res;
    //also make changes in EGR_tune

}

double Algorithms::EGr_tune(double a, double b, double c, double d, double e)
{
    //QString test_alg="bully";//egreedy,satisF,ql
    QString test_alg="bully";//markov
    QVector<QString> opponents={"best_resp","determined","godfather","maxmin","nash","TFT"};
    double sum_results=0.0;
    int num_results=0;

    //for all games
    for(int i=0;i<widget->G_data.count();i++)
    {
        IterParam pars;
        pars.plnum=0;
        pars.game=&widget->G_data[i];
        boolean sym=false;//symmetric game?
        if(widget->comp_data.symmetric_games.contains(pars.game->Generator))sym=true;

        //for all opponents
        for(int j=0;j<opponents.count();j++)
        {
            QVector<QString> players;
            players.append(opponents[j]);
            players.append(test_alg);
            sum_results+=run_mini_comparison(players,test_alg,pars,a,b,c,d,e);
            if(sym==false)//assymmetric games
            {
                players.insert(1,players.takeFirst());
                sum_results+=run_mini_comparison(players,test_alg,pars,a,b,c,d,e);
                num_results++;
            }
            num_results++;
        }

        //selfplay
        QVector<QString> players(2,test_alg);
        sum_results+=run_mini_comparison(players,test_alg,pars,a,b,c,d,e);

        num_results++;
        //bully tune
        qDebug()<<pars.game->Generator<<"B"<<sum_results;
        //qDebug()<<"sumrew_game"<<sum_results;
        qDebug()<<"nresults"<<num_results;
    }
    qDebug()<<"sum results"<<sum_results<<num_results;
    return sum_results/num_results;
}



double Algorithms::run_mini_comparison(QVector<QString> players,QString test_alg,IterParam pars,double MWin,double Ql_disc, double Ql_expl, double Ql_dec_l, double Ql_dec_expl)
{//
    //double Sat_aspir,double Sat_pers_rate
    qDebug()<<"players"<<players;
    double average_rew=0;

    int pl0_max=INT_MIN,pl1_max=INT_MIN;
    int pl0_min=INT_MAX,pl1_min=INT_MAX;
    for(int i= 0; i< pars.game->Matrix_i.count(); i++)//normalise rewards
    {
        if(pars.game->Matrix_i[i][0]>pl0_max)pl0_max=pars.game->Matrix_i[i][0]; //pl0 max
        if(pars.game->Matrix_i[i][0]<pl0_min)pl0_min=pars.game->Matrix_i[i][0]; //pl0 min
        if(pars.game->Matrix_i[i][1]>pl1_max)pl1_max=pars.game->Matrix_i[i][1]; //pl1 max
        if(pars.game->Matrix_i[i][1]<pl1_min)pl1_min=pars.game->Matrix_i[i][1]; //pl1 min
    }

    for(int round=0;round<widget->comp_data.Num_rounds;round++)//one round
    {
        int num_test_alg=0;
        QVector<int> tot_rew(players.count(),0);
        QVector<Comp_algo*> algos;
        QVector<int> actions,rewards;
        int last_rounds= widget->comp_data.Num_iteration-widget->comp_data.Last_x_rounds;
        for(int i=0;i<players.count();i++)//init algorithms
        {
            pars.plnum=i;
            actions.append(0);
            if (players[i]=="best_resp"){algos.append(new Best_resp()); }
            else if (players[i]=="bully"){algos.append(new Bully_own()); }//   Bully(&pars)
            else if(players[i]=="determined"){algos.append(new Determ(&pars));}
            //else if (players[i]=="egreedy"){algos.append(new Egreedy(&pars,static_cast<int>(expl_rate))); }
            else if(players[i]=="fict_play"){algos.append(new Fict_pl(&pars)); }
            else if(players[i]=="godfather"){algos.append(new Godfather(&pars)); }
            else if(players[i]=="markov"){algos.append(new Markov(&pars,static_cast<int>(MWin))); }//comp_data->Mark_win
            else if(players[i]=="maxmin"){algos.append(new Maxmin(&pars)); }
            else if(players[i]=="nash"){algos.append(new Nash(&pars)); }
            else if(players[i]=="ngreedy"){algos.append(new Ngreedy(&pars)); }
            else if(players[i]=="noreg"){algos.append(new Noreg(&pars)); }
            else if(players[i]=="pareto_opt"){algos.append(new Pareto_opt(&pars,widget->comp_data.Par_comp));}
            //else if(players[i]=="ql"){algos.append(new QL(&pars,Ql_lear,Ql_disc, Ql_expl, Ql_dec_l, Ql_dec_expl)); }
            else if(players[i]=="random")algos.append(new Random());
            //else if(players[i]=="satisF"){algos.append(new SatisF(Sat_aspir,Sat_pers_rate)); }
            else if(players[i]=="TFT"){algos.append(new TitforTat(&pars,widget->comp_data.TFT_smart)); }

            if(players[i]==test_alg)num_test_alg+=1;
        }

        for(int iter=0;iter<widget->comp_data.Num_iteration;iter++)//one iteration
        {
            pars.past_act=actions;
            pars.iteration=iter;
            for(int i=0;i<players.count();i++)//get actions
            {
                 pars.plnum=i;
                 actions[i]=algos[i]->action(&pars);
            }
            int cel=pars.game->Calculate_bincel(actions,pars.game->Stepsize);
            pars.past_rews=pars.game->Matrix_i[cel];

            if(iter>=last_rounds)
            {
                for(int i=0;i<players.count();i++){tot_rew[i]+=pars.game->Matrix_i[cel][i];}
            }
        }
        //qDebug()<<"tot_rews"<<tot_rew;
        //qDebug()<<"devider"<<widget->comp_data.Last_x_rounds*widget->comp_data.Num_rounds*num_test_alg;

        if(players[0]==test_alg)
        {
            double rew=static_cast<double>(tot_rew[0])/(widget->comp_data.Last_x_rounds*widget->comp_data.Num_rounds*num_test_alg);
            //qDebug()<<"r1"<<rew<<pl0_min<<pl0_max<<((rew-pl0_min)/(pl0_max-pl0_min));
            average_rew+=((rew-pl0_min)/(pl0_max-pl0_min));
        }
        if(players[1]==test_alg)
        {
            double rew=static_cast<double>(tot_rew[1])/(widget->comp_data.Last_x_rounds*widget->comp_data.Num_rounds*num_test_alg);
            //qDebug()<<"r2"<<rew<<pl1_min<<pl1_max<<((rew-pl1_min)/(pl1_max-pl1_min));
            average_rew+=((rew-pl1_min)/(pl1_max-pl1_min));
        }

        //for(int i=0;i<players.count();i++){if(players[i]==test_alg){average_rew+=static_cast<double>(tot_rew[i])/(widget->comp_data.Last_x_rounds*widget->comp_data.Num_rounds*num_test_alg);}}
        //qDebug()<<"average_rew"<<average_rew;
    }

    return average_rew;

}
