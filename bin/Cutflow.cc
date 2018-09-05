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
   
   
   printf("%40s     %10s    %6s    %16s\n","cut","n events","ratio wrt first","ratio wrt previous");
   std::string delimiter = "::";
   int firstbin = -1;
   for ( int i = 1; i <= h ->GetNbinsX(); ++i )
   {
      std::string binlabel = std::string(h->GetXaxis()->GetBinLabel(i));
      if ( binlabel == "" ) continue;
      if ( firstbin < 0 ) firstbin = i;
      std::string label = binlabel.substr(binlabel.find(delimiter)+ delimiter.length());
      float n = h->GetBinContent(i);
      float rn1 = h->GetBinContent(i)/h->GetBinContent(firstbin);
      float rni = 0;
      if ( i == firstbin )
      {
         printf("%40s:    %10.0f    %1.4f               - \n",label.c_str(),n,rn1);
      }
      else
      {
         rni = h-> GetBinContent(i)/h->GetBinContent(i-1);
         printf("%40s:    %10.0f    %1.4f               %1.4f \n",label.c_str(),n,rn1,rni);
      }
      
   }
   
   return 0;
   
}

