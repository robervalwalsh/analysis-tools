#ifndef Analysis_Tools_Utils_h
#define Analysis_Tools_Utils_h 1

namespace analysis {
   namespace tools {

      struct FilterResults
      {
         int   total;
         int   filtered;
         float efficiency;
      };
      
      struct PDF
      {
         std::pair<int,int> id;
         std::pair<double,double> x;
      };

      template <typename T> 
      struct less_than_pt
      {
          inline bool operator() (const T& gp1, const T& gp2)
          {
              return (gp1.pt() < gp2.pt());
          }
      };
      
      template <typename T> 
      struct greater_than_pt
      {
          inline bool operator() (const T& gp1, const T& gp2)
          {
              return (gp1.pt() > gp2.pt());
          }
      };
      
   }
}




#endif  // Analysis_Tools_Utils_h
