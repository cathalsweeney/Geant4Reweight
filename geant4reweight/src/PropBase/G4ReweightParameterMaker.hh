#ifndef G4ReweightParameterMaker_h
#define G4ReweightParameterMaker_h

#include "TH1D.h"

#include <map>
#include <vector>

#include "fhiclcpp/ParameterSet.h"

#include "geant4reweight/src/util/FitParameter.hh"


class G4ReweightParameterMaker{

  public:
    G4ReweightParameterMaker(){};
    G4ReweightParameterMaker( const std::map< std::string, std::vector< FitParameter > > & pars );
    G4ReweightParameterMaker( const std::vector< fhicl::ParameterSet > & FitParSets, bool doProton=false );
    void SetNewVals( const std::vector< std::pair< std::string, double > > & input );
    void SetNewVals( const std::map< std::string, double > & input );
    void SetNewValsWithElast(const std::vector<std::pair<std::string,double>> &input  , const std::vector<std::pair<std::string,double>> &input_elast );
void SetNewValsWithElast( const std::map< std::string, double > & input ,   const std::map< std::string, double > & input_elast  );    


void SetParamVals( const std::map< std::string, double > & input );

//set parameter values with one set for reac and one set for elast
void SetParamValsWithElast( const std::map< std::string, double > & input ,  const std::map< std::string, double > & input_elast);

    void BuildHistsFromPars();
    void BuildElasticHist();
    const std::map< std::string, TH1D* > & GetFSHists() const { return FSHists; };
    TH1D * GetElasticHist(){ return ElasticHist; };

    const std::map< std::string, std::vector< FitParameter > > & GetParameterSet() const { return FullParameterSet; };

//added by C Thorpe
	const std::vector< FitParameter > GetElasticParameterSet() const { return ElasticParameterSet; };


    //Return as a vector for setting values within FitManager
    std::vector< std::pair< std::string, double > > GetParametersAsPairs(){
      std::vector< std::pair< std::string, double > > results;

      for( auto itPar = FullParameterSet.begin(); itPar != FullParameterSet.end(); ++itPar ){
        for( size_t i = 0; i < itPar->second.size(); ++i ){
          if( !itPar->second.at(i).Dummy ){

//std::cout << "Parameter Pair: " << itPar->
            results.push_back( std::make_pair( itPar->second.at(i).Name, itPar->second.at(i).Value ) ); 
          }
        }
      }

      return results;
    };

    std::map< std::string, double > GetActiveParameterValues(){

     std::map< std::string, double > results;

      for( auto itPar = FullParameterSet.begin(); itPar != FullParameterSet.end(); ++itPar ){
        for( size_t i = 0; i < itPar->second.size(); ++i ){
          if( !itPar->second.at(i).Dummy ){
            results.insert( std::make_pair( itPar->second.at(i).Name, itPar->second.at(i).Value ) ); 
          }
        }       
      }

      return results;
    };

    std::map< std::string, FitParameter > GetActiveParameters(){

     std::map< std::string, FitParameter > results;

      for( auto itPar = FullParameterSet.begin(); itPar != FullParameterSet.end(); ++itPar ){
        for( size_t i = 0; i < itPar->second.size(); ++i ){
          if( !itPar->second.at(i).Dummy ){
            results.insert( std::make_pair( itPar->second.at(i).Name, itPar->second.at(i) ) ); 
          }
        }       
      }

      return results;
    };

    std::vector< std::pair< std::string, FitParameter > > GetActiveParametersAsPairs(){

      std::vector< std::pair< std::string, FitParameter > > results;

      for( auto itPar = FullParameterSet.begin(); itPar != FullParameterSet.end(); ++itPar ){
        for( size_t i = 0; i < itPar->second.size(); ++i ){
          if( !itPar->second.at(i).Dummy ){
            results.push_back( std::make_pair( itPar->second.at(i).Name, itPar->second.at(i) ) ); 
          }
        }       
      }

      return results;
    };
    int GetNParameters(){ return nParameters; };
    //added by C thorpe
    int GetNElastParameters(){ return nElastParameters; };

  protected:

    std::map< std::string, TH1D* > FSHists;
    TH1D * dummyHist;
    std::map< std::string, std::vector< FitParameter > > FullParameterSet;
    std::vector< FitParameter > ElasticParameterSet;

    TH1D* ElasticHist;


    int nParameters;
//added by C Thorpe
   int nElastParameters;

};

#endif
