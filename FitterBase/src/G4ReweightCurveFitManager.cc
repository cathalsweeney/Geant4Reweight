#include "G4ReweightCurveFitManager.hh"
#include "newDUETFitter.hh"
#include "TVectorD.h"
#include "TTree.h"

G4ReweightCurveFitManager::G4ReweightCurveFitManager(std::string & fOutFileName) {
  out = new TFile( fOutFileName.c_str(), "RECREATE" );
  data_dir = out->mkdir( "Data" );
}

void G4ReweightCurveFitManager::MakeFitParameters( std::vector< fhicl::ParameterSet > & FitParSets ){
  for( size_t i = 0; i < FitParSets.size(); ++i ){
    fhicl::ParameterSet theSet = FitParSets.at(i);
    std::string theCut = theSet.get< std::string >("Cut");

    if( FullParameterSet.find( theCut ) == FullParameterSet.end() ){  
      FullParameterSet[ theCut ] = std::vector< FitParameter  >();
    }   

    bool isDummy = theSet.get< bool >("Dummy");
    if( isDummy ){
      FitParameter dummyPar;
      dummyPar.Name = "dummy";
      dummyPar.Cut = theCut;
      dummyPar.Value = 1.; 
      dummyPar.Range = std::make_pair( 0., 0.);
      dummyPar.Dummy = true;
          
      FullParameterSet[ theCut ].push_back( dummyPar );
      CutIsDummy[ theCut ] = true;
    }   
    else{ 
      std::cout << "Making parameters for " << theCut << std::endl;
        
      std::vector< fhicl::ParameterSet > theParameters = theSet.get< std::vector< fhicl::ParameterSet > >("Parameters");
      for( size_t j = 0; j < theParameters.size(); ++j ){
        fhicl::ParameterSet thePar = theParameters.at(j);


        std::string theName = thePar.get< std::string >("Name");
        std::cout << theName << std::endl;
            
        std::pair< double, double > theRange = thePar.get< std::pair< double, double > >("Range");
        std::cout << "Range Low: " << theRange.first << " High: " << theRange.second << std::endl;

        FitParameter par;
        par.Name = theName;
        par.Cut = theCut;
        par.Dummy = false;
        par.Value = 1.; 
        par.Range = theRange;
        FullParameterSet[ theCut ].push_back( par );
      }   
      CutIsDummy[ theCut ] = false;
    }   
  } 
}

void G4ReweightCurveFitManager::DefineMCSets( std::vector< fhicl::ParameterSet > &MCSets ){

  for( size_t i = 0; i < MCSets.size(); ++i ){
    std::string theSet = MCSets[i].get< std::string >("Name");
    sets.push_back( theSet );
    mapSetsToFracs[ theSet ] = MCSets[i].get< std::string >( "FSFile" );
    mapSetsToNominal[ theSet ] = MCSets[i].get< std::string >( "File" );
  }

}

void G4ReweightCurveFitManager::DefineExperiments( fhicl::ParameterSet &ps){
  std::vector< fhicl::ParameterSet > exps = ps.get< std::vector< fhicl::ParameterSet > >("Experiments");
  std::cout << "Getting Experiments: "  << exps.size() << std::endl;
  for(size_t i = 0; i < exps.size(); ++i){
    std::cout << std::endl;
    std::cout << "Trying to find " << exps.at(i).get<std::string>("Name") << " " << exps.at(i).get<std::string>("Type") << std::endl;
    
    if( IsSetActive( exps.at(i).get<std::string >( "Type" ) ) ){
      G4ReweightFitter * exp = new G4ReweightFitter(out, exps.at(i));
      mapSetsToFitters[ exp->GetType() ].push_back( exp );
    }
  }

  if( IsSetActive( "C_piplus" ) ){
    bool includeDUET = ps.get< bool >("IncludeDUET");
    std::string DUET_data = ps.get< std::string >( "DUETDataFile" ); 
    newDUETFitter * df = new newDUETFitter(out, DUET_data);
    if( includeDUET ){
      std::cout << "Including DUET" << std::endl;
      mapSetsToFitters["C_piplus"].push_back( df );
    }
  }
}

void G4ReweightCurveFitManager::GetAllData(){
  std::map< std::string, std::vector< G4ReweightFitter* > >::iterator
    itSet = mapSetsToFitters.begin();
  for( ; itSet != mapSetsToFitters.end(); ++itSet ){

    std::cout << "Loading Data for Set: " << itSet->first;
    std::vector< G4ReweightFitter* > fitters = itSet->second;
    for( size_t i = 0; i < fitters.size(); ++i ){
      fitters[i]->LoadData();
      fitters[i]->SaveData(data_dir);
      allFitters.push_back( fitters[i] );
    }
  }

}


void G4ReweightCurveFitManager::RunFitAndSave( ){
  //Create Fit Tree to store the chi2 values and parameters
  TTree fit_tree( "FitTree", "");
  double chi2 = 0.;
  fit_tree.Branch( "Chi2", &chi2 );


  std::cout << "Nominal: " << std::endl;
  std::map< std::string, std::vector< FitParameter > >::iterator it;
  for( it = FullParameterSet.begin(); it != FullParameterSet.end(); ++it ){
    std::cout << it->first << std::endl;
    for( size_t i = 0; i < it->second.size(); ++i ){
      std::cout << "\t" << it->second.at(i).Name << " " << it->second.at(i).Value << std::endl;
    }
  }

  std::map< std::string, double > parameter_values;
  std::vector< std::string > thePars = {"fAbsHigh", "fAbsLow"};
  for( int i = 0; i < thePars.size(); ++i ){
    std::string branch_name = thePars.at(i);
    std::cout << "Making branch for " << branch_name << std::endl;

    parameter_values[ branch_name ] = 0.;
    fit_tree.Branch( branch_name.c_str(), &parameter_values.at( branch_name ), (branch_name + "/D").c_str() );   
  }

  for( int k = 0; k < 5; ++k ){
    std::vector< double > theVals = {1.0, 1.0};
    theVals[0] += .1 * k;
    std::string dir_name = "";

    for(size_t i = 0; i < thePars.size(); ++i){
      parameter_values[ thePars.at(i) ] = theVals[i];

      std::map< std::string, std::vector< FitParameter > >::iterator it;
      for( it = FullParameterSet.begin(); it != FullParameterSet.end(); ++it ){
        std::cout << it->first << std::endl;
        for( size_t j = 0; j < it->second.size(); ++j ){
          if( it->second[j].Name == thePars[i] ){
            it->second[j].Value = theVals[i];
            dir_name += ( it->second[j].Name + std::to_string(it->second[j].Value) );
          }
          std::cout << "\t" << it->second.at(j).Name << " " << it->second.at(j).Value << std::endl;
        }
      }   
    }

    TDirectory * outdir = out->mkdir( dir_name.c_str() );

    double chi2 = 0.;
    std::map< std::string, std::vector< G4ReweightFitter* > >::iterator
      itSet = mapSetsToFitters.begin();

    for( ; itSet != mapSetsToFitters.end(); ++itSet ){
      std::cout << itSet->first << std::endl;
      for( size_t i = 0; i < itSet->second.size(); ++i ){
        auto theFitter = itSet->second.at(i); 
        std::cout << "Fitter: " << theFitter->GetName() << std::endl;

        std::string NominalFile = mapSetsToNominal[ itSet->first ];
        std::string FracsFile = mapSetsToFracs[ itSet->first ];
        std::cout << NominalFile << " " << FracsFile << std::endl;

        theFitter->MakeFitDir( outdir );
        theFitter->GetMCFromCurves( NominalFile, FracsFile, FullParameterSet);
        double fit_chi2 = theFitter->DoFit();

        std::cout << fit_chi2 << std::endl;

        chi2 += fit_chi2;

        theFitter->FinishUp();

      }
    }
    fit_tree.Fill();
  }
   
  out->cd();
  fit_tree.Write();
  out->Close();
  std::cout << "Done" << std::endl;
 
  
}

