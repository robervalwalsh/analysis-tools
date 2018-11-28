#include <string>
#include <iostream>
#include <vector>

#include "TFile.h" 
#include "TH1.h" 


// =============================================================================================   
int main(int argc, char * argv[])
{
   if ( argc <= 1 ) return -1;
   
   const char * rootfile = argv[1];
   
   TFile f(rootfile,"old");
   
   TH1F * h = (TH1F*) f.Get("cutflow");
   
   
   printf("+%s+\n", std::string(110,'-').c_str());
   printf("| %-48s |    %10s |   %16s |   %16s |\n","cut","n events","ratio wrt first","ratio wrt previous");
   printf("+%s+\n", std::string(110,'-').c_str());
   int firstbin = -1;
   for ( int i = 1; i <= h ->GetNbinsX(); ++i )
   {
      std::string label = std::string(h->GetXaxis()->GetBinLabel(i));
      if ( label == "" ) continue;
      if ( firstbin < 0 ) firstbin = i;
      float n = h->GetBinContent(i);
      float rn1 = h->GetBinContent(i)/h->GetBinContent(firstbin);
      float rni = 0;
      if ( i == firstbin )
      {
         printf("| %-48s |    %10.0f |   %16.4f |  %19s |\n",label.c_str(),n,rn1,"-");
      }
      else
      {
         rni = h-> GetBinContent(i)/h->GetBinContent(i-1);
         printf("| %-48s |    %10.0f |   %16.4f |     %16.4f |\n",label.c_str(),n,rn1,rni);
      }
      
   }
   printf("+%s+\n", std::string(110,'-').c_str());

   return 0;
   
}

