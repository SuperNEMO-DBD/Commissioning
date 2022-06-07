// simplified version of sndisplay (contains only sndisplay::demonstrator)
//                           `-> https://github.com/emchauve/sndisplay

#ifndef SNDISPLAY_CC
#define SNDISPLAY_CC

#include "TBox.h"
#include "TCanvas.h"
#include "TColor.h"
#include "TEllipse.h"
#include "TH2D.h"
#include "TLine.h"
#include "TPaletteAxis.h"
#include "TRandom.h"
#include "TSystem.h"
#include "TString.h"
#include "TText.h"

#include<vector>

namespace sndisplay
{
  class palette
  {
  private:
    palette()
    {
      const Int_t nRGBs = 6;
      Double_t stops[nRGBs] = { 0.00, 0.20, 0.40, 0.60, 0.80, 1.00 };
      Double_t red[nRGBs]   = { 0.25, 0.00, 0.20, 1.00, 1.00, 0.90 };
      Double_t green[nRGBs] = { 0.25, 0.80, 1.00, 1.00, 0.80, 0.00 };
      Double_t blue[nRGBs]  = { 1.00, 1.00, 0.20, 0.00, 0.00, 0.00 };

      palette_index = TColor::CreateGradientColorTable(nRGBs, stops, red, green, blue, 100);
    }

    static palette *instance;
    int palette_index;

  public:
    ~palette() {};

    static palette *get_me() {
      if (instance == nullptr) instance = new palette;
      return instance;}

    static int get_index() {
      return get_me()->palette_index;}

  }; // sndisplay::palette class

  palette *palette::instance = nullptr;

  /////////////////////////////
  // sndisplay::demonstrator //
  /////////////////////////////

  class demonstrator
  {
  public:
    demonstrator (const char *n = "") : demonstrator_name (n)
    {
      canvas = nullptr;

      range_min = range_max = -1;

      // TOP_VIEW //

      const double spacerx = 0.005;
      const double spacery = 0.025;

      const double title_sizey = 0.0615;

      const double mw_sizey = (1-2*spacery-title_sizey)/(2.0 + 4*1.035 + 0.125);
      const double xw_sizey = 1.035*mw_sizey;
      const double se_sizey = 0.125*mw_sizey;
      const double gg_sizey = (1-2*spacery-title_sizey-2*mw_sizey-se_sizey)/18.0;

      const double mw_sizex = (1-2*spacerx)/(20 + 2*0.5*0.720);
      const double xw_sizex = (1-2*spacerx-20*mw_sizex);
      const double se_sizex = (1-2*spacerx-2*xw_sizex);
      const double gg_sizex = se_sizex/113.0;

      // printf("gg_sizex = %f\n", gg_sizex);
      // printf("gg_sizey = %f\n", gg_sizey);

      // MW (column only)

      for (int mw_side=0; mw_side<2; ++mw_side) {

	for (int mw_column=0; mw_column<20; ++mw_column) {

	  double x1 = spacerx + 0.5*xw_sizex + mw_column*mw_sizex;
	  double y1 = spacery + (1-mw_side)*(mw_sizey+4*xw_sizey+se_sizey);

	  double x2 = x1 + mw_sizex;

	  double y2 = y1 + mw_sizey;

	  top_om_content.push_back(0);

	  TBox *box = new TBox(x1, y1, x2, y2);
	  box->SetFillColor(0);
	  box->SetLineWidth(1);
	  top_om_box.push_back(box);
	  
	  TString omid_string = Form("M:%1d.%d.*", mw_side, mw_column);
	  TText *omid_text = new TText (x1+0.5*mw_sizex, y1+0.667*mw_sizey, omid_string);
	  omid_text->SetTextSize(0.032);
	  omid_text->SetTextAlign(22);
	  top_om_text.push_back(omid_text);

	  // TText *content_text = new TText (x1+0.5*mw_sizex, y1+0.333*mw_sizey, "");
	  // content_text->SetTextSize(0.02);
	  // content_text->SetTextAlign(22);
	  // content_text_v.push_back(content_text);

	} // for mw_column
	
      } // for mw_side

      // XW (column only)

      for (int xw_side=0; xw_side<2; ++xw_side) {

	for (int xw_wall=0; xw_wall<2; ++xw_wall) {

	  for (int xw_column=0; xw_column<2; ++xw_column) {

	    double x1 = spacerx + xw_wall*(xw_sizex+113*gg_sizex);
	    double x2 = x1 + xw_sizex;

	    double y1 = spacery + mw_sizey;

	    if (xw_side == 0)
	      y1 += 2*xw_sizey + se_sizey + xw_column*xw_sizey;
	    else y1 += (1-xw_column)*xw_sizey;

	    double y2 = y1 + xw_sizey;

	    top_om_content.push_back(0);

	    TBox *box = new TBox(x1, y1, x2, y2);
	    box->SetFillColor(0);
	    box->SetLineWidth(1);
	    top_om_box.push_back(box);
	  
	    TString omid_string = Form("X:%1d.%1d.%1d.*", xw_side, xw_wall, xw_column);
	    TText *omid_text = new TText (x1+0.5*xw_sizex, y1+0.6*xw_sizey, omid_string);
	    omid_text->SetTextSize(0.032);
	    omid_text->SetTextAlign(22);
	    top_om_text.push_back(omid_text);

	  }
	}
      }

      for (int gg_side=0; gg_side<2; ++gg_side) {

	  for (int gg_row=0; gg_row<113; ++gg_row) {

	    for (int gg_layer=0; gg_layer<9; ++gg_layer)
	      {
		double x1 = spacerx + xw_sizex + gg_row*gg_sizex;
		double y1 = spacery + mw_sizey;

		if (gg_side == 0)
		  y1 += 9*gg_sizey + se_sizey + gg_layer*gg_sizey;
		else
		  y1 += (8-gg_layer)*gg_sizey;

		double x2 = x1 + gg_sizex;
		double y2 = y1 + gg_sizey;

		top_gg_content.push_back(0);

		TBox *box = new TBox(x1, y1, x2, y2);
		box->SetFillColor(0);
		box->SetLineWidth(1);
		top_gg_box.push_back(box);

		TEllipse *ellipse = new TEllipse((x1+x2)/2, (y1+y2)/2, gg_sizex/2, gg_sizey/2);
		ellipse->SetFillColor(0);
		ellipse->SetLineWidth(1);
		top_gg_ellipse.push_back(ellipse);
	      }
	  }
      }

      title = new TText (spacerx, 1-title_sizey*3/4, "");
      title->SetTextSize(0.056);
      title->SetTextAlign(12);

    } // demonstrator ()


    void setrange(float zmin, float zmax) 
    {
      range_min = zmin; range_max = zmax;
    }


    void draw_top()
    {
      update(false);

      if (canvas == nullptr)
	{
	  const int canvas_width  = 1600;
	  const int canvas_height = 400; // 374 without title

	  canvas = new TCanvas (Form("C_demonstrator_%s",demonstrator_name.Data()), Form("%s",demonstrator_name.Data()), canvas_width, canvas_height);

	  // force canvas exact size
	  int decoration_width = canvas_width - canvas->GetWw();
	  int decoration_height = canvas_height - canvas->GetWh();
	  canvas->SetWindowSize(canvas_width+decoration_width, canvas_height+decoration_height);

	  // preserve width/height ratio in case of resizing
	  canvas->SetFixedAspectRatio();
	}
      else canvas->cd();

      for (int mw_side=0; mw_side<2; ++mw_side)
	{
	  for (int mw_column=0; mw_column<20; ++mw_column)
	    {
	      int top_om_num = mw_side*20 + mw_column;
 	      top_om_box[top_om_num]->Draw("l");
	      top_om_text[top_om_num]->Draw();
	    }
	}

      for (int xw_side=0; xw_side<2; ++xw_side)
	{
	  for (int xw_wall=0; xw_wall<2; ++xw_wall)
	    {
	      for (int xw_column=0; xw_column<2; ++xw_column)
		{
		  int top_om_num = 40 + xw_side*2*2 + xw_wall*2 + xw_column;
		  top_om_box[top_om_num]->Draw("l");
		  top_om_text[top_om_num]->Draw();
		}
	    }
	}
      
      for (int gg_side=0; gg_side<2; ++gg_side)
	{
	  for (int gg_row=0; gg_row<113; ++gg_row)
	    {
	      for (int gg_layer=0; gg_layer<9; ++gg_layer)
		{
		  int top_gg_num = gg_side*113*9 + gg_row*9 + gg_layer;
		  top_gg_box[top_gg_num]->Draw("l");
		  top_gg_ellipse[top_gg_num]->Draw("l");
		  // top_g_text[top_gg_num]->Draw();
		}
	    }
	}

      title->Draw();

    } // draw_top

    void setomcontent (int om_num, float value)
    {
      int top_om_num = -1;
      
      if (om_num < 260) // MW IT
	{
	  int om_side = 0;
	  int om_column = (om_num/13);
	  top_om_num = om_side*20 + om_column;
	}
      else if (om_num < 520) // MW IT
	{
	  int om_side = 1;
	  int om_column = (om_num-260)/13;
	  top_om_num = om_side*20 + om_column;
	}
      else if (om_num < 648) // XW
	{
	  int om_side = (om_num < 584) ? 0 : 1;
	  int om_wall = (om_num-520-om_side*64)/32;
	  int om_column = (om_num-520-om_side*64-om_wall*32)/16;
	  top_om_num = 40 + om_side*2*2 + om_wall*2 + om_column;
	}

      top_om_content[top_om_num] = value;
    }


    void setggcontent (int cell_num, float value)
    {
      if (cell_num < 2034) top_gg_content[cell_num] = value;
      else printf("*** wrong cell ID\n");
    }

    float getggcontent (int cell_num)
    {
      return top_gg_content[cell_num];
    }
    
    void setggcontent (int cell_side, int cell_row, int cell_layer, float value)
    {
      int cell_num = cell_side*9*113 + cell_row*9 + cell_layer;
      setggcontent(cell_num, value);
    }
    
    void setggcolor (int cell_num, Color_t color)
    {
      if (cell_num < 2034) top_gg_ellipse[cell_num]->SetFillColor(color);
      else printf("*** wrong cell ID\n");
    }
    
    void setggcolor (int cell_side, int cell_row, int cell_layer, Color_t color)
    {
      int cell_num = cell_side*9*113 + cell_row*9 + cell_layer;
      setggcolor(cell_num, color);
    }

    void settitle (const char *text)
    {
      title->SetText(title->GetX(), title->GetY(), text);
    }
    
    void reset ()
    {
      for (size_t om=0; om<top_om_content.size(); ++om)
	{
	  top_om_content[om] = 0;
	  top_om_box[om]->SetFillColor(0);
	}

      for (size_t gg=0; gg<top_gg_content.size(); ++gg)
	{
	  top_gg_content[gg] = 0;
	  top_gg_ellipse[gg]->SetFillColor(0);
	  // top_gg_box[gg]->SetFillColor(0);
	}
    }

    void update_canvas ()
    {
      canvas->Modified();
      canvas->Update();

      gSystem->ProcessEvents();
    }

    void update (bool update_canvas_too=true)
    {
      float top_content_min = top_om_content[0];
      float top_content_max = top_om_content[0];

      for (size_t om=0; om<top_om_content.size(); ++om)
    	{
    	  if (top_om_content[om] < top_content_min) top_content_min = top_om_content[om];
    	  if (top_om_content[om] > top_content_max) top_content_max = top_om_content[om];
    	}

      for (size_t gg=0; gg<top_gg_content.size(); ++gg)
    	{
    	  if (top_gg_content[gg] < top_content_min) top_content_min = top_gg_content[gg];
    	  if (top_gg_content[gg] > top_content_max) top_content_max = top_gg_content[gg];
    	}

      top_content_min = 0;
      if (range_min != -1) top_content_min = range_min;
      if (range_max != -1) top_content_max = range_max;
      // printf("Z range = [%f, %f] for '%s'\n", top_content_min, top_content_max, demonstrator_name.Data());

      for (size_t om=0; om<top_om_content.size(); ++om)
    	{
    	  if (top_om_content[om] != 0)
    	    {
    	      int color_index = floor (99*(top_om_content[om]-top_content_min)/(top_content_max-top_content_min));
    	      if (color_index < 0) color_index = 0;
    	      else if (color_index >= 100) color_index = 99;
	      top_om_box[om]->SetFillColor(palette::get_index() + color_index);
    	    }
    	  else
    	    top_om_box[om]->SetFillColor(0);
    	}

      for (size_t gg=0; gg<top_gg_content.size(); ++gg)
    	{
    	  if (top_gg_content[gg] != 0)
    	    {
    	      int color_index = floor (99*(top_gg_content[gg]-top_content_min)/(top_content_max-top_content_min));
    	      if (color_index < 0) color_index = 0;
    	      else if (color_index >= 100) color_index = 99;
	      top_gg_ellipse[gg]->SetFillColor(palette::get_index() + color_index);
    	    }
    	  else
    	    top_gg_ellipse[gg]->SetFillColor(0);
    	}

      if (update_canvas_too)
	update_canvas();
    }

    //

    TString demonstrator_name;

    TCanvas *canvas;
    TText *title;

    float range_min, range_max;
    
    std::vector<float> top_om_content;
    std::vector<TBox*> top_om_box;
    std::vector<TText*> top_om_text;

    std::vector<float> top_gg_content;
    std::vector<TBox*> top_gg_box;
    std::vector<TEllipse*> top_gg_ellipse;
    // std::vector<TText*>  top_gg_text;

  }; // sndisplay::demonstrator class

} // sndisplay namespace

#endif // SNDISPLAY_CC
