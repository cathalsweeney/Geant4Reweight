#include "Geant4/G4HadronInelasticProcess.hh"
#include "Geant4/G4HadronElasticProcess.hh"
#include "Geant4/G4PionPlus.hh"
#include "Geant4/G4PionMinus.hh"
#include "Geant4/G4Proton.hh"
#include "Geant4/G4ParticleDefinition.hh"
#include "Geant4/G4DynamicParticle.hh"
#include "Geant4/G4ThreeVector.hh"
#include "Geant4/G4Track.hh"
#include "Geant4/G4Step.hh"
#include "Geant4/G4StepPoint.hh"
#include "Geant4/G4Material.hh"
#include "Geant4/G4SystemOfUnits.hh"
#include "Geant4/G4ProcessVector.hh"
#include "Geant4/G4ProcessManager.hh"
#include "Geant4/G4RunManager.hh"
#include "Geant4/G4VParticleChange.hh"
//#include "Geant/G4bool.hh"

#include "Geant4/G4SteppingManager.hh"


#include "geant4reweight/src/PredictionBase/G4CascadeDetectorConstruction.hh"
#include "geant4reweight/src/PredictionBase/G4CascadePhysicsList.hh"

#include <utility>
#include <iostream>
#include <fstream>

#include "TH1F.h"
#include "TFile.h"
#include "TTree.h"
#include "TGraph.h"
#include "TVectorD.h"

#include "TMath.h"


#include "TH2D.h"

#include "fhiclcpp/make_ParameterSet.h"
#include "fhiclcpp/ParameterSet.h"

//#ifdef FNAL_FHICL
#include "cetlib/filepath_maker.h"
//#endif

std::string fcl_file;

//bool range_override = false;
//double range_low_override = 0.;
//double range_high_override = 0.;

std::string output_file_override = "empty"; 

//int ncasc_override = 0;
//int ndiv_override = 0;
//int type_override = -999;


struct Config{

  Config(){};
 Config( fhicl::ParameterSet & pset ){
    nCascades = pset.get< size_t >("NCascades");
 //   if( ncasc_override > 0 ) 
 //     nCascades = ncasc_override;

    type      = pset.get< int >("Type");
 //   if( type_override != -999 )
 //     type = type_override;

//    range = pset.get< std::pair< double, double > >("Range");
//    if( range_override) 
//      range = std::make_pair(range_low_override, range_high_override);


//    nDivisions = pset.get< size_t >("NDivisions");
//    if( ndiv_override > 0 ) 
//      nDivisions = ndiv_override;

    outFileName = pset.get< std::string >("Outfile");
  //   if( output_file_override  != "empty" ){
  //    outFileName = output_file_override;
  //  }

    fhicl::ParameterSet MaterialParameters = pset.get< fhicl::ParameterSet >("Material");
    MaterialName = MaterialParameters.get< std::string >( "Name" );
    MaterialZ = MaterialParameters.get< int >( "Z" );
    MaterialMass = MaterialParameters.get< double >( "Mass" );
    MaterialDensity = MaterialParameters.get< double >( "Density" );

Energy = pset.get<double>("Energy");
//ScatteringAngle = pset.get<double>("ScatteringAngle");
BinningAngle = pset.get<double>("BinningAngle");
BinningEnergy = pset.get<double>("BinningEnergy");

CrossSecFile = pset.get<std::string>("CrossSecFile");

  };

  size_t nCascades; 
//  size_t nDivisions;
  int type;
//  std::pair< double, double > range;

  std::string outFileName;

  int MaterialZ;
  double MaterialMass;
  double MaterialDensity;
  std::string MaterialName;

//incident particle energy energy
  double Energy;
//scattering angle to look at
double ScatteringAngle;

double BinningAngle;
double BinningEnergy;

std::string CrossSecFile;

};

struct TrackStepPart{
  G4Track * theTrack;
  G4Step * theStep;
  G4DynamicParticle * dynamic_part;
};

Config configure(fhicl::ParameterSet & pset);
std::vector< double > fillMomenta(Config theConfig);
TrackStepPart initTrackAndPart_withSM(G4ParticleDefinition * part_def, G4Material * theMaterial, G4SteppingManager &stepman );
TrackStepPart initTrackAndPart(G4ParticleDefinition * part_def, G4Material * theMaterial);



bool parseArgs(int argc, char* argv[]);
void initRunMan( G4RunManager & rm );
void makeFCLParameterSet( fhicl::ParameterSet & pset);
G4HadronInelasticProcess * getInelasticProc( /*G4HadronInelasticProcess * inelastic_proc, */G4ParticleDefinition * part_def, std::string inel_name );
G4HadronElasticProcess * getElasticProc( /*G4HadronInelasticProcess * inelastic_proc, */G4ParticleDefinition * part_def, std::string el_name );
void CheckAllProcs(G4ParticleDefinition * part_def);


////////// Main is Here ////////////////////



int main(int argc, char * argv[]){

std::cout << "starting" << std::endl;

  if( !parseArgs(argc, argv) ) 
    return 0;

  fhicl::ParameterSet pset;
  makeFCLParameterSet(pset);

std::cout << "Params made" << std::endl;

  Config theConfig( pset );

std::cout << "Config made" << std::endl;


/*
  if( theConfig.nDivisions < 1 ){
    std::cout << "Please give NDivision >= 1" << std::endl;
    return 0;
  }
  if( theConfig.range.second < theConfig.range.first ){
    std::cout << "Please give range in increasing order" << std::endl;
    return 0;
  }
*/


//std::cout << theConfig.CrossSecFile << std::endl;


TFile *f_CrossSection = TFile::Open(theConfig.CrossSecFile.c_str());

TGraph *g_inel;
TGraph *g_el;

f_CrossSection->GetObject("inel_KE",g_inel);
f_CrossSection->GetObject("el_KE",g_el);

f_CrossSection->Close();

///  std::vector< double > momenta = fillMomenta( theConfig );

  TFile * fout = new TFile( theConfig.outFileName.c_str(), "RECREATE");

  TVectorD m_vec(1);
  m_vec[0] = theConfig.MaterialMass;
  m_vec.Write("Mass");

  TVectorD d_vec(1);
  d_vec[0] = theConfig.MaterialDensity;
  d_vec.Write("Density");

  std::cout << "Got mass & density" << std::endl;

/*
  TTree * tree = new TTree("tree","");  
  int nPi0 = 0, nPiPlus = 0, nPiMinus = 0, nProton, nNeutron;
  double momentum;
  tree->Branch( "nPi0", &nPi0 );
  tree->Branch( "nPiPlus", &nPiPlus );
  tree->Branch( "nPiMinus", &nPiMinus );
  tree->Branch( "nProton", &nProton );
  tree->Branch( "nNeutron", &nNeutron );
  tree->Branch( "momentum", &momentum );

*/

  std::cout << "Initializing" << std::endl;
  //Initializing
  G4RunManager rm;
  initRunMan( rm );
 

 G4SteppingManager sm;

 G4SteppingManager sm_elast;

  std::cout << "Initialized" << std::endl;

  G4PionPlus  * piplus = 0x0;
  G4PionMinus * piminus = 0x0;
  G4Proton    * proton = 0x0;
  G4Neutron   * neutron = 0x0;
  G4ParticleDefinition * part_def = 0x0;
  G4ParticleDefinition * part_def_elast = 0x0;  
std::string inel_name = "";
  std::string el_name = ""; 

switch( theConfig.type ){
    case 211:
      std::cout << "Chose PiPlus" << std::endl;
      part_def = piplus->Definition();
      part_def_elast = piplus->Definition();
      inel_name = "pi+Inelastic";
     el_name = "hadElastic"; 
	break;
    
    case -211:
      std::cout << "Chose PiMinus" << std::endl;
      part_def = piminus->Definition();
	part_def_elast = piminus->Definition();

      inel_name = "pi-Inelastic";
	el_name = "hadElastic";
      break;

    case 2212: 
    {
      std::cout << "Chose Proton" << std::endl;
      part_def = proton->Definition();
	part_def_elast = proton->Definition();
     
 inel_name = "protonInelastic";
	el_name = "hadElastic";

      //Default for now
 //     ++momenta.back(); 
 //     std::vector< double > total_ys(momenta.size(), 1.);
 //     TGraph total_gr(momenta.size(), &momenta[0], &total_ys[0]);
 //     fout->cd();
 //     total_gr.Write( "total" );
 //     fout->Close();
 //     //delete rm;
//      return 1;
    
      //Returns before this, but... eh
      break;
    }

    case 2112:
    {
      std::cout << "Chose Neutron" << std::endl;
      part_def = neutron->Definition();
	part_def_elast = neutron->Definition();
     
 inel_name = "neutronInelastic";
      el_name = "hadElastic";
      //Default for now
  //    ++momenta.back(); 
  //    std::vector< double > total_ys(momenta.size(), 1.);
   //   TGraph total_gr(momenta.size(), &momenta[0], &total_ys[0]);
  //    fout->cd();
  //    total_gr.Write( "total" );
  //    fout->Close();
      //delete rm;
 //     return 1;
    
      //Returns before this, but... eh
      break;
      
    }
    

    default:
      std::cout << "Please specify either 211, -211, 2112, or 2212" << std::endl;
      fout->cd();
      fout->Close();
      //delete rm;
      return 0;
      
  }



  //G4HadronInelasticProcess * inelastic_proc = 0x0;
//G4HadronElasticProcess   * elastic_proc = getElasticProc(part_def , el_name);


  //std::cout << "inelastic_proc: " << inelastic_proc << std::endl;
  //G4HadronInelasticProcess * inelastic_proc = 0x0;
                 


//  std::cout << "Elastic: " << elastic_proc << std::endl;

/*
if(!elastic_proc){
return 0;
}
*/


  G4Material * theMaterial = new G4Material(theConfig.MaterialName, theConfig.MaterialZ, theConfig.MaterialMass*g/mole, theConfig.MaterialDensity*g/cm3);





//run the cascade to do inelastic scatter

//std::cout << theMomentum << std::endl;
    double KE = theConfig.Energy;
//    double theMomentum = sqrt(2*part_def->GetPDGMass()*KE + KE*KE);
//std::cout  << theMomentum << std::endl;

//std::cout << theConfig.nCascades << std::endl;
//double momentum;
double Energy; //outgoing particle KE
double sin_theta; //sine of outgoing particle angle 


int n_Angle_bins = 2 / theConfig.BinningAngle;
int n_Energy_bins = (1000-0)/theConfig.BinningEnergy;


TH2D *h_inel = new TH2D("h_inel","h_inel",n_Energy_bins,0.,1000,n_Angle_bins,0,3.1415);
TH2D *h_el = new TH2D("h_el","h_el",n_Energy_bins,0.,1000,n_Angle_bins,0,3.1415);

h_inel->GetXaxis()->SetTitle("Outgoing P KE (MeV)");
h_inel->GetYaxis()->SetTitle("Outgoing P #theta (rad)");
//h_inel->GetZaxis()->SetTitle("d#sigma / d#theta d E_{k} (mb/sr/MeV)");

h_el->GetXaxis()->SetTitle("Outgoing P KE (MeV)");
h_el->GetYaxis()->SetTitle("Outgoing P #theta (rad)");
//h_el->GetZaxis()->SetTitle("d#sigma / d#theta d E_{k} (mb/sr/MeV)");


bool elastic = true;
bool inelastic = true;


if(elastic){

std::cout << "Doing Elastic" << std::endl;
  
    CheckAllProcs(part_def);
  
      G4HadronElasticProcess * elastic_proc = getElasticProc(part_def_elast, "hadElastic");




  
        if (!elastic_proc) {
  
            std::cout << "Could not find elastic process" << std::endl;
  
              return 0;
                   }
  
 
  elastic_proc->SetVerboseLevel(0);

  elastic_proc->ProcessDescription(std::cout);





  TrackStepPart track_par = initTrackAndPart_withSM( part_def, theMaterial , sm );
 
 G4Track * theTrack = track_par.theTrack;
  G4Step * theStep   = track_par.theStep;
  G4DynamicParticle * dynamic_part = track_par.dynamic_part;



    dynamic_part->SetKineticEnergy( KE );

    for( size_t iC = 0; iC < theConfig.nCascades; ++iC ){

 //     momentum = dynamic_part->GetTotalMomentum();
     if( !(iC % 10000) ) std::cout << "\tCascade: " << iC << std::endl;


//old components of 4 momentum
//double oldx,oldy,oldz;

//oldx = theTrack->GetMomentumDirection().x();
//oldy = theTrack->GetMomentumDirection().y();
//oldz = theTrack->GetMomentumDirection().z();


    //  std::cout << "Old x: " << theTrack_elast->GetMomentumDirection().x() << std::endl;
  //    std::cout << "Old y: " << theTrack_elast->GetMomentumDirection().y() << std::endl;
//      std::cout << "Old z: " << theTrack_elast->GetMomentumDirection().z() << std::endl;

G4ParticleChange * thePC = (G4ParticleChange*)elastic_proc->PostStepDoIt( *theTrack, *theStep );	

double newx,newy,newz;

Energy = thePC->GetEnergy();
//std::cout << "Outgoing hadron energy:  " << Energy << std::endl;

newx = thePC->GetMomentumDirection()->x();
newy = thePC->GetMomentumDirection()->y();
newz = thePC->GetMomentumDirection()->z();

sin_theta = sqrt(newx*newx + newy*newy)*newz/sqrt(newz*newz);

double theta = TMath::ASin(sin_theta);

if(sin_theta < 0) theta += 3.1415;

//std::cout  << theta << "  "  << Energy << std::endl;

     // size_t nSecondaries = thePC->GetNumberOfSecondaries();

       thePC->SetVerboseLevel(0);

thePC->Initialize(*theTrack);

h_el->Fill(Energy,theta);



}

//fill histogram



std::cout << h_el->Integral() << std::endl;

//normalize histogram to 1 * (1-abs probability) 

h_el->Scale(1.0 / theConfig.nCascades);
h_el->Scale(1.0 / theConfig.BinningAngle);
h_el->Scale(1.0 / theConfig.BinningEnergy);

std::cout << h_el->Integral("width") << std::endl;

h_el->Scale(g_el->Eval(theConfig.Energy));

std::cout << h_el->Integral("width") << std::endl;

std::string histname = "el_" + std::to_string((int)theConfig.Energy) + "_MeV";

h_el->Write(histname.c_str());

}

if(inelastic){

std::cout << "Doing Inelastic" << std::endl;
  G4HadronInelasticProcess * inelastic_proc = getInelasticProc( /*inelastic_proc, */part_def, inel_name );

  if( !inelastic_proc ){
    //delete rm;
    return 0;
  }




  TrackStepPart track_par = initTrackAndPart( part_def, theMaterial  );
 
 G4Track * theTrack = track_par.theTrack;
  G4Step * theStep   = track_par.theStep;
  G4DynamicParticle * dynamic_part = track_par.dynamic_part;

    dynamic_part->SetKineticEnergy( KE );





//initialize everythin again
//G4VParticleChange * thePC;
//thePC->Initialize(*theTrack);



for( size_t iC = 0; iC < theConfig.nCascades; ++iC ){

//std::cout << "Cascade " << iC << std::endl;

     if( !(iC % 10000) ) std::cout << "\tCascade: " << iC << std::endl;

//std::cout << "Doing Inelastic" << std::endl;
//
      //momentum = dynamic_part->GetTotalMomentum();
  G4VParticleChange *   thePC = (G4VParticleChange*)inelastic_proc->PostStepDoIt( *theTrack, *theStep );

      size_t nSecondaries = thePC->GetNumberOfSecondaries();

//std::cout << "Num Inel Secondaries: " << nSecondaries << std::endl;

double outgoing_KE = 0;


//std::cout << "NSecondaries: " << nSecondaries << std::endl;

//first proton is always leading proton (I think)
      for( size_t i = 0; i < nSecondaries; ++i ){

        auto part = thePC->GetSecondary(i)->GetDynamicParticle();

//std::cout << part->GetPDGcode() << "    " << part->GetKineticEnergy() << std::endl;
if(part->GetPDGcode() == theConfig.type){


outgoing_KE = part->GetKineticEnergy();

 
sin_theta =sqrt( part->GetMomentumDirection().getX()*part->GetMomentumDirection().getX() + part->GetMomentumDirection().getY()*part->GetMomentumDirection().getY()) * part->GetMomentumDirection().getZ() / sqrt( part->GetMomentumDirection().getZ()*part->GetMomentumDirection().getZ());

//scattering angle in rad
double theta = TMath::ASin(sin_theta);

if(sin_theta < 0) theta += 3.1415;


h_inel->Fill(outgoing_KE,theta);

}

//find highest momentum proton





}
      thePC->SetVerboseLevel(0);

thePC->Initialize(*theTrack);


}


std::cout << h_inel->Integral() << std::endl;

//normalize histogram to 1 * (1-abs probability) 


h_inel->Scale(1.0 / theConfig.nCascades);
h_inel->Scale(1.0 / theConfig.BinningAngle);
h_inel->Scale(1.0 / theConfig.BinningEnergy);

std::cout << h_inel->Integral("width") << std::endl;



//h_inel->Scale(1.0/h_inel->Integral("width"));

h_inel->Scale(g_inel->Eval(theConfig.Energy));

std::cout << h_inel->Integral("width") << std::endl;




std::string histname = "inel_" + std::to_string((int)theConfig.Energy) + "_MeV";

h_inel->Write(histname.c_str());


}


TH2D *h_tot = (TH2D*)h_inel->Clone("h_tot");
h_tot->Add(h_el);


std::string histname = "tot_" + std::to_string((int)theConfig.Energy) + "_MeV";
h_tot->Write(histname.c_str());

  fout->Close();
  //delete rm;

 
 return 0;
}


bool parseArgs(int argc, char ** argv){

  bool found_fcl_file = false;

//  bool found_range_low = false;
//  bool found_range_high = false;

  for( int i = 1; i < argc; ++i ){
    if( ( strcmp( argv[i], "--help" )  == 0 ) || ( strcmp( argv[i], "-h" ) == 0 ) ){
      std::cout << "Usage: ./G4Cascade -c <cascade_config>.fcl [options]" << std::endl;
      std::cout << std::endl;
      std::cout << "Options: " << std::endl;
      std::cout << "\t-o <output_file_override>.root" << std::endl;
      std::cout << "\t--low <range_low_value> (must be provided along with -high)" << std::endl; 
      std::cout << "\t--high <range_high_value> (must be provided along with -low)" << std::endl; 
      std::cout << "\t--NC <number_of_cascades_per_point> (must be > 0 to work)" << std::endl;
      std::cout << "\t--ND <divisor_of_range> (must be > 0 to work)" << std::endl;
      std::cout << "\t-t <probe type> (currently only works with 211 and -211)" << std::endl;

      return false;
    }

    else if( strcmp( argv[i], "-c" ) == 0 ){
      fcl_file = argv[i+1];
      found_fcl_file = true;
    }
/*
    else if( strcmp( argv[i], "-o" ) == 0 ){
      output_file_override = argv[i+1];
    }

    else if( strcmp( argv[i], "--low" ) == 0 ){
      range_low_override = atof( argv[i+1] ); 
      found_range_low = true;
    }

    else if( strcmp( argv[i], "--high" ) == 0 ){
      range_high_override = atof( argv[i+1] ); 
      found_range_high = true;
    }

    else if( strcmp( argv[i], "--NC" ) == 0 ){
      ncasc_override = atoi( argv[i+1] ); 
    }

    else if( strcmp( argv[i], "--ND" ) == 0 ){
      ndiv_override = atoi( argv[i+1] ); 
    }

    else if( strcmp( argv[i], "-t" ) == 0 ){
      type_override = atoi( argv[i+1] );
    }
*/  
}

  if( !found_fcl_file ){
    std::cout << "Error: Must provide fcl file with option '-c'" << std::endl;
    return false;
  }

//  if( found_range_low != found_range_high ){
//    std::cout << "Error: If overriding fcl range, you must provide both values with options '-l' and '-h'" << std::endl;
//    return false;
//  }
//  else{
//    range_override = found_range_low;
//  }

  

  return true;
}




void initRunMan( G4RunManager & rm ){
  rm.SetUserInitialization(new G4CascadeDetectorConstruction);
  rm.SetUserInitialization(new G4CascadePhysicsList);
  rm.Initialize();
  rm.ConfirmBeamOnCondition();
  rm.ConstructScoringWorlds();
  rm.RunInitialization();
}

void makeFCLParameterSet( fhicl::ParameterSet & pset){
  // Configuration file lookup policy.
  char const* fhicl_env = getenv("FHICL_FILE_PATH");
  std::string search_path;

  if (fhicl_env == nullptr) {
    std::cerr << "Expected environment variable FHICL_FILE_PATH is missing or empty: using \".\"\n";
    search_path = ".";
  }
  else {
    search_path = std::string{fhicl_env};
  }

//std::cout << search_path << std::endl;

  cet::filepath_first_absolute_or_lookup_with_dot lookupPolicy{search_path};

  fhicl::make_ParameterSet(fcl_file, lookupPolicy, pset);
}

/*void getInelasticProc( G4HadronInelasticProcess * inelastic_proc, G4ParticleDefinition * part_def, std::string inel_name ){

  G4ProcessManager * pm = part_def->GetProcessManager();
  G4ProcessVector  * pv = pm->GetProcessList();
  
  for( int i = 0; i < pv->size(); ++i ){
    G4VProcess * proc = (*pv)(i);
    std::string theName = proc->GetProcessName();
    std::cout <<  theName << std::endl;
    if( theName == inel_name ){
      std::cout << "Found inelastic" << std::endl;
      inelastic_proc = (G4HadronInelasticProcess*)proc;
    }
  }
}
*/


G4HadronInelasticProcess * getInelasticProc( G4ParticleDefinition * part_def, std::string inel_name ){

  G4ProcessManager * pm = part_def->GetProcessManager();
  G4ProcessVector  * pv = pm->GetProcessList();
  
  for( int i = 0; i < pv->size(); ++i ){
    G4VProcess * proc = (*pv)(i);
    std::string theName = proc->GetProcessName();
    std::cout <<  theName << std::endl;
    if( theName == inel_name ){
      std::cout << "Found inelastic" << std::endl;
      return (G4HadronInelasticProcess*)proc;
    }
  }
  return 0x0;
}


/*
G4HadronElasticProcess * getElasticProc( G4ParticleDefinition * part_def, std::string el_name ){

  G4ProcessManager * pm = part_def->GetProcessManager();
  G4ProcessVector  * pv = pm->GetProcessList();
  
  for( int i = 0; i < pv->size(); ++i ){
    G4VProcess * proc = (*pv)(i);
    std::string theName = proc->GetProcessName();
    std::cout <<  theName << std::endl;
    if( theName == el_name ){
      std::cout << "Found elastic" << std::endl;
      return (G4HadronElasticProcess*)proc;
    }
  }
  return 0x0;
}
*/


G4HadronElasticProcess * getElasticProc(G4ParticleDefinition * part_def, std::string el_name) {

  G4ProcessManager * pm = part_def->GetProcessManager();

  G4ProcessVector  * pv = pm->GetProcessList();

  for( int i = 0; i < pv->size(); ++i ){

    G4VProcess * proc = (*pv)(i);

    std::string theName = proc->GetProcessName();

    std::cout <<  theName << std::endl;

    if (theName == el_name) {

    std::cout << "Found elastic" << std::endl;

      //return (G4HadronElasticProcess*)proc;
      
            return dynamic_cast<G4HadronElasticProcess *>(proc);
      
                }
      
                  }
      
                    return 0x0;
     
                    }
      




Config configure(fhicl::ParameterSet & pset){
  Config theConfig;

 // theConfig.nCascades = pset.get< size_t >("NCascades");
 // if( ncasc_override > 0 ) 
 //   theConfig.nCascades = ncasc_override;

  theConfig.type      = pset.get< int >("Type");
//  if( type_override != -999 )
//    theConfig.type = type_override;

  theConfig.outFileName = pset.get< std::string >("Outfile");
   if( output_file_override  != "empty" ){
    theConfig.outFileName = output_file_override;
  }

  fhicl::ParameterSet MaterialParameters = pset.get< fhicl::ParameterSet >("Material");
  theConfig.MaterialName = MaterialParameters.get< std::string >( "Name" );
  theConfig.MaterialZ = MaterialParameters.get< int >( "Z" );
  theConfig.MaterialMass = MaterialParameters.get< double >( "Mass" );
  theConfig.MaterialDensity = MaterialParameters.get< double >( "Density" );


  return theConfig;
}




TrackStepPart initTrackAndPart_withSM(G4ParticleDefinition * part_def, G4Material * theMaterial, G4SteppingManager & stepman){

  G4DynamicParticle * dynamic_part = new G4DynamicParticle(part_def, G4ThreeVector(0.,0.,1.), 0. );

  std::cout << "PDG: " << dynamic_part->GetPDGcode() << std::endl;

  G4Track * theTrack = new G4Track( dynamic_part, 0., G4ThreeVector(0.,0.,0.) );

  G4Step * theStep = new G4Step();

  G4StepPoint * thePoint = new G4StepPoint();

  thePoint->SetMaterial( theMaterial );

  theStep->SetPreStepPoint( thePoint );

  theTrack->SetStep( theStep );

  //new
  //
    stepman.SetInitialStep(theTrack);
  
      theStep->InitializeStep(theTrack);
 
       ///////
  
         TrackStepPart results;
 
           results.theTrack = theTrack;
  
             results.theStep = theStep;
 
               results.dynamic_part = dynamic_part;
 
                return results;
 
              }
 









TrackStepPart initTrackAndPart(G4ParticleDefinition * part_def, G4Material * theMaterial ){
  G4DynamicParticle * dynamic_part = new G4DynamicParticle(part_def, G4ThreeVector(0.,0.,1.), 0. );
  std::cout << "PDG: " << dynamic_part->GetPDGcode() << std::endl;

  G4Track * theTrack = new G4Track( dynamic_part, 0., G4ThreeVector(0.,0.,0.) );
  G4Step * theStep = new G4Step();
  G4StepPoint * thePoint = new G4StepPoint();
  thePoint->SetMaterial( theMaterial );
  theStep->SetPreStepPoint( thePoint );
  theTrack->SetStep( theStep );

  TrackStepPart results;
  results.theTrack = theTrack;
  results.theStep = theStep;
  results.dynamic_part = dynamic_part;
  return results;
}






void CheckAllProcs(G4ParticleDefinition * part_def) {

  G4ProcessManager * pm = part_def->GetProcessManager();

  G4ProcessVector  * pv = pm->GetProcessList();

  

  for (int i = 0; i < pv->size(); ++i) {

    G4VProcess * proc = (*pv)(i);

    std::string theName = proc->GetProcessName();

    std::cout <<  theName << std::endl;

  }

}

