#pragma once
void render_button(void* a) {
    // get parent valwindow from *a
    // get values from edit controls on parent window
    // render manelbrot to copy
    // paste buffer
    // draw buffer to window
    cout << "render" << endl;
    ValWindow* manwind = WindowMan.fromName("mandelbrot controller");
    string xcoord = manwind->eval_edit("x_coord");
    string ycoord = manwind->eval_edit("y_coord");
    string wcoord = manwind->eval_edit("w_coord");
    string hcoord = manwind->eval_edit("h_coord");
    string itts = manwind->eval_edit("itterations");
    SHOW(xcoord);
    SHOW(ycoord);
    SHOW(wcoord);
    SHOW(hcoord);
    SHOW(itts);
    //cout <<xcoord <<endl;
    int type = VC_CHECKBOX;
    bool wrong_mand = manwind->evaluate_controlb("do_wrong_mand", type);
    cout << 1 << endl;
    double x = strtodbl(xcoord);
    cout << 2 << endl;
    double y = strtodbl(ycoord);
    cout << 3 << endl;
    double w = strtodbl(wcoord);
    cout << 4 << endl;
    double h = strtodbl(hcoord);
    cout << 5 << endl;
    double it = strtodbl(itts);
    SHOW(x);
    SHOW(y);
    SHOW(w);
    SHOW(h);
    SHOW(it);
    manwind = WindowMan.fromName("mandelbrot");
    if (wrong_mand) manwind->mygfx.draw_mandelbrot_tocopy_wrong(x, y, w, h, it);
    else  manwind->mygfx.draw_mandelbrot_tocopy(x, y, w, h, it);

    cout << "mand drawn" << endl;
    manwind->mygfx.paste_buf();
    manwind->mygfx.save_image("mandtest.bmp");
    manwind->GFXdraw();
    //cout << manwind->evaluate_controlb("do_wrong_mand", type)<< "  type: " ;
    //cout << type << endl;

}

void zoombutt(void* a) {
    ValWindow* manwind = WindowMan.fromName("mandelbrot controller");
    string xcoord = manwind->eval_edit("x_coord");
    string ycoord = manwind->eval_edit("y_coord");
    string wcoord = manwind->eval_edit("w_coord");
    string hcoord = manwind->eval_edit("h_coord");
    string zoomfa = manwind->eval_edit("zoom_factor");
    double x = strtodbl(xcoord);
    double y = strtodbl(ycoord);
    double w = strtodbl(wcoord);
    double h = strtodbl(hcoord);
    double zf = strtodbl(zoomfa);
    double cr = w * zf;
    w -= cr;
    x += cr / 2;
    cr = h * zf;
    h -= cr;
    y += cr / 2;

    //x /= 1.01;
    //y /= 1.01;
    //w /= 1.01;
    //h /= 1.01;

    manwind->set_editctrl_text("x_coord", ctostr(x));
    manwind->set_editctrl_text("y_coord", ctostr(y));
    manwind->set_editctrl_text("h_coord", ctostr(h));
    manwind->set_editctrl_text("w_coord", ctostr(w));
}
void move_mand(int dir) {
    ValWindow* manwind = WindowMan.fromName("mandelbrot controller");
    string xcoord = manwind->eval_edit("x_coord");
    double hold, shold;
    double fact = strtodbl(manwind->eval_edit("move_factor"));
    if (dir < 3) {
        hold = strtodbl(manwind->eval_edit("x_coord"));
        shold = strtodbl(manwind->eval_edit("w_coord"));

        if (dir == 1)hold -= shold * fact;
        if (dir == 2)hold += shold * fact;
        manwind->set_editctrl_text("x_coord", ctostr(hold));
    }
    if (dir > 2) {
        hold = strtodbl(manwind->eval_edit("y_coord"));
        shold = strtodbl(manwind->eval_edit("h_coord"));

        if (dir == 3)hold -= shold * fact;
        if (dir == 4)hold += shold * fact;
        manwind->set_editctrl_text("y_coord", ctostr(hold));
    }
}
void leftbutt(void* a) { move_mand(1); }
void rightbutt(void* a) { move_mand(2); }
void upbutt(void* a) { move_mand(3); }
void downbutt(void* a) { move_mand(4); }
void savebutt(void* a) {
    ValWindow* manwind = WindowMan.fromName("mandelbrot controller");
    WindowMan.fromName("mandelbrot")->mygfx.save_image(manwind->eval_edit("savefilename") + ".bmp");
}
void do_mandelbrot_thread(void* a) {
    /*wrong test coords for examples:
    fractal edge- stars -.6 -> -.609 : -.5559  : .02 : .035
    in the crack (switch x/y/wrong) .2501 -.000022 .0000875 .00004575



    */
    // creaet window
    // add controls to window- x,y,w,h,itts
    // add wrong render checkbox
    // add render button
    // add status functionality

    //ValWindow* vw = WindowMan.create_new_windowex("mandelbrot", 100, 100, 1000, 1000, 1);
    ValWindow* vw = WindowMan.create_new_windowts("mandelbrot", 100, 100, 1000, 1000, 1);
    int controllerw = 600, controllerh = 600;
    //ValWindow* vwc = WindowMan.create_new_windowex("mandelbrot controller", 100, 800, controllerw,controllerh,1);
    ValWindow* vwc = WindowMan.create_new_windowts("mandelbrot controller", 100, 800, controllerw, controllerh, 1);
    string xcord = "-3";// "-0.5892348415";
    string ycord = "-3";//"-0.570152343242";
    string wcord = "6";//"0.0200000065456001";
    string hcord = "6";//"0.0350000545600000001";
    string itts = "512";
    //vwc->add_control_ex_ts(VC_BUTTON, "render_button", "Render", VRECT{ 25,225,100,40 }, (void*)&render_button, true);
    //vwc->add_control_ex_ts(VC_BUTTON, "zoom_button", "zoom", VRECT{ 25,275,100,40 }, (void*)&zoombutt, true);
    //vwc->add_editctrl_ts("zoom_factor", VRECT{ 60,330,80,25 }, ".05");
    //vwc->add_control_ex_ts(VC_LABEL, "zoom_label", "zoom %: ", VRECT{ 5,330,50,25 }, VC_NOFUNC, false);
    //vwc->add_editctrl_ts("move_factor", VRECT{ 220,330,80,25 }, ".05");

    //vwc->add_control_ex_ts(VC_LABEL, "move_label", "move %: ", VRECT{ 150,330,60,25 }, VC_NOFUNC, false);

    //vwc->add_control_ex_ts(VC_BUTTON, "left_button", "left", VRECT{ 170,255,35,25 }, (void*)&leftbutt, true);
    //vwc->add_control_ex_ts(VC_BUTTON, "right_button", "right", VRECT{ 260,255,35,25 }, (void*)&rightbutt, true);
    //vwc->add_control_ex_ts(VC_BUTTON, "up_button", "up", VRECT{ 215,230,35,25 }, (void*)&upbutt, true);
    //vwc->add_control_ex_ts(VC_BUTTON, "down_button", "down", VRECT{ 215,280,35,25 }, (void*)&downbutt, true);

    //vwc->add_editctrl_ts("x_coord", VRECT{ 70,20,200,25 }, xcord);
    //vwc->add_control_ex_ts(VC_LABEL, "xcordlabel", "X: ", VRECT{ 5,20,50,25 }, VC_NOFUNC, false);
    //vwc->add_editctrl_ts("y_coord", VRECT{ 70,50,200,25 }, ycord);
    //vwc->add_control_ex_ts(VC_LABEL, "ycordlabel", "y: ", VRECT{ 20,50,50,25 }, VC_NOFUNC, false);
    //vwc->add_editctrl_ts("w_coord", VRECT{ 70,80,200,25 }, wcord);
    //vwc->add_control_ex_ts(VC_LABEL, "wcordlabel", "Width: ", VRECT{ 5,80,50,25 }, VC_NOFUNC, false);
    //vwc->add_editctrl_ts("h_coord", VRECT{ 70,110,200,25 }, hcord);
    //vwc->add_control_ex_ts(VC_LABEL, "hcordlabel", "Height: ", VRECT{ 5,110,50,25 }, VC_NOFUNC, false);
    //vwc->add_editctrl_ts("itterations", VRECT{ 70,140,200,25 }, itts);
    //vwc->add_control_ex_ts(VC_LABEL, "itts_label", "itterations: ", VRECT{ 5,140,50,25 }, VC_NOFUNC, false);

    //vwc->add_editctrl_ts("savefilename", VRECT{ 370,50,100,25 }, "mand");
    //vwc->add_control_ex_ts(VC_LABEL, "savelabel", ".bmp", VRECT{ 470,50,50,25 }, VC_NOFUNC, false);
    //vwc->add_control_ex_ts(VC_LABEL, "savelabel", "save file: ", VRECT{ 310,50,50,25 }, VC_NOFUNC, false);
    //vwc->add_control_ex_ts(VC_BUTTON, "save_button", "save", VRECT{ 320,80,60,25 }, (void*)&savebutt, true);
    //vwc->add_control_ex_ts(VC_CHECKBOX, "do_wrong_mand", "use wrong addition", VRECT{ 5,180,150,25 }, VC_NOFUNC, false);

    ValWindow* wind = vwc;
    VWM_register_datan(render_button, "render_button");
    VWM_register_datan(zoombutt, "zoombutt");
    VWM_register_datan(leftbutt, "leftbutt");
    VWM_register_datan(rightbutt, "rightbutt");

    VWM_register_datan(upbutt, "upbutt");
    VWM_register_datan(downbutt, "downbutt");

    VWM_register_datan(savebutt, "savebutt");

    wind->AddControl("title=render_button,type=VC_BUTTON,text=Render,x=25,y=225,w=100,h=40,needsparent=true,dataname=render_button");
    wind->AddControl("title=zoom_button,type=VC_BUTTON,text=zoom,x=25,y=275,w=100,h=40,needsparent=true,dataname=zoombutt");
    wind->AddControl("title=zoom_factor,type=VC_EDIT,text=.05,x=60,y=330,w=80,h=25,numblurs=0,transparent=true,nointeract=true");
    wind->AddControl("title=zoom_label,type=VC_LABEL,text=zoom %: ,x=5,y=330,w=50,h=25,numblurs=1,transparent=true");
    wind->AddControl("title=move_factor,type=VC_EDIT,text=.05,x=250,y=330,w=80,h=25,numblurs=0,transparent=true,nointeract=true");
    wind->AddControl("title=move_label,type=VC_LABEL,text=move %: ,x=180,y=330,w=60,h=25,numblurs=1,transparent=true");
    wind->AddControl("title=left_button,type=VC_BUTTON,text=left,x=200,y=255,w=35,h=25,needsparent=true,dataname=leftbutt");
    wind->AddControl("title=right_button,type=VC_BUTTON,text=right,x=290,y=255,w=35,h=25,needsparent=true,dataname=rightbutt");
    wind->AddControl("title=up_button,type=VC_BUTTON,text=up,x=245,y=230,w=35,h=25,needsparent=true,dataname=upbutt");
    wind->AddControl("title=down_button,type=VC_BUTTON,text=down,x=245,y=280,w=35,h=25,needsparent=true,dataname=downbutt");
    wind->AddControl("title=x_coord,type=VC_EDIT,text=-3,x=90,y=30,w=200,h=25,numblurs=0,transparent=true,nointeract=true");
    wind->AddControl("title=xcordlabel,type=VC_LABEL,text=X: ,x=25,y=30,w=50,h=25,numblurs=1,transparent=true");
    wind->AddControl("title=y_coord,type=VC_EDIT,text=-3,x=90,y=60,w=200,h=25,numblurs=0,transparent=true,nointeract=true");
    wind->AddControl("title=ycordlabel,type=VC_LABEL,text=y: ,x=40,y=60,w=50,h=25,numblurs=1,transparent=true");
    wind->AddControl("title=w_coord,type=VC_EDIT,text=6,x=90,y=90,w=200,h=25,numblurs=0,transparent=true,nointeract=true");
    wind->AddControl("title=wcordlabel,type=VC_LABEL,text=Width: ,x=25,y=90,w=50,h=25,numblurs=1,transparent=true");
    wind->AddControl("title=h_coord,type=VC_EDIT,text=6,x=90,y=120,w=200,h=25,numblurs=0,transparent=true,nointeract=true");
    wind->AddControl("title=hcordlabel,type=VC_LABEL,text=Height: ,x=25,y=120,w=50,h=25,numblurs=1,transparent=true");
    wind->AddControl("title=itterations,type=VC_EDIT,text=512,x=90,y=150,w=200,h=25,numblurs=0,transparent=true,nointeract=true");
    wind->AddControl("title=itts_label,type=VC_LABEL,text=itterations: ,x=25,y=150,w=50,h=25,numblurs=1,transparent=true");
    wind->AddControl("title=savefilename,type=VC_EDIT,text=mand,x=390,y=50,w=100,h=25,numblurs=0,transparent=true,nointeract=true");
    wind->AddControl("title=savelabel2,type=VC_LABEL,text=.bmp,x=490,y=50,w=50,h=25,numblurs=1,transparent=true");
    wind->AddControl("title=savelabel,type=VC_LABEL,text=save file: ,x=300,y=50,w=80,h=25,numblurs=1,transparent=true");
    wind->AddControl("title=save_button,type=VC_BUTTON,text=save,x=530,y=50,w=60,h=25,needsparent=true,dataname=savebutt");
    wind->AddControl("title=do_wrong_mand,type=VC_CHECKBOX,text=use wrong addition,x=25,y=190,w=250,h=25");

    VWM_register_datar(render_button, "render_button");
    VWM_register_datar(zoombutt, "zoombutt");
    VWM_register_datar(leftbutt, "leftbutt");
    VWM_register_datar(rightbutt, "rightbutt");
    VWM_register_datar(upbutt, "upbutt");
    VWM_register_datar(downbutt, "downbutt");
    VWM_register_datar(savebutt, "savebutt");


}