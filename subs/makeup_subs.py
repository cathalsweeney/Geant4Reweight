energies = ["50","100","150","200",
            "250","300","400","500",
            "600","700","800", "1000", "1200"]

unit = "MeV"            

samps_to_vars = {"nom":"-i 1. -e 1.", 
                 "var_inel1.5_elast1": "-i 1.5 -e 1.",
                 "var_inel1_elast1.5": "-i 1. -e 1.5",
                 "var_inel1.5_elast1.5": "-i 1.5 -e 1.5"}
                 

for samp in ["nom","var_inel1.5_elast1","var_inel1_elast1.5","var_inel1.5_elast1.5"]:
  for e in energies:
    infile = file("sub_base.txt","r")
    outFileName = "Sim/"+samp[0:3] + "/" + "GeantReweight_makeup_" + e + unit + "_" + samp + ".sh" 
    outfile = file(outFileName, "w")
    for line in infile:
      if "./G4Thin" in line:
        newline = "./G4Sim -c ${_CONDOR_SCRATCH_DIR}/code/macros/macros/"+e+unit+"/"+e+unit+"_${MAKEUP}_"+samp[0:3] +".mac -o Ar_"+samp+"_"+e+unit+"_${MAKEUP}.root " + samps_to_vars[samp] 
        outfile.write(newline)
      elif "ifdh cp thin" in line:
        newline = "ifdh mkdir /pnfs/dune/scratch/users/calcuttj/GeantReweight/Sim/${JOBSUBJOBID:0:10}/\n"
        outfile.write(newline)
        newline = "ifdh cp Ar_"+samp+"_"+e+unit+"_${MAKEUP}.root /pnfs/dune/scratch/users/calcuttj/GeantReweight/Sim/${JOBSUBJOBID:0:10}/Ar_"+samp+"_"+e+unit+"_${MAKEUP}.root\n" 
        outfile.write(newline)
      else:
        outfile.write(line)
    newline = "cd ${_CONDOR_SCRATCH_DIR}/code/ReweightBase-build/\n" 
    outfile.write(newline)

    if "nom" in samp: 
      newline = "./G4Reweight -f ${_CONDOR_SCRATCH_DIR}/code/G4Sim-build/Ar_nom_"+e+unit+"_${MAKEUP}.root -o Ar_"+e+unit+"_nom_inel1.5_elast1_${MAKEUP}.root -i 1.5 -e 1. \n"
      outfile.write(newline)

      newline = "ifdh cp Ar_"+e+unit+"_nom_inel1.5_elast1_${MAKEUP}.root /pnfs/dune/scratch/users/calcuttj/GeantReweight/Sim/${JOBSUBJOBID:0:10}/Ar_"+e+unit+"_nom_inel1.5_elast1_${MAKEUP}.root\n"  
      outfile.write(newline)

      newline = "./G4Reweight -f ${_CONDOR_SCRATCH_DIR}/code/G4Sim-build/Ar_nom_"+e+unit+"_${MAKEUP}.root -o Ar_"+e+unit+"_nom_inel1_elast1.5_${MAKEUP}.root -i 1. -e 1.5 \n"
      outfile.write(newline)

      newline = "ifdh cp Ar_"+e+unit+"_nom_inel1_elast1.5_${MAKEUP}.root /pnfs/dune/scratch/users/calcuttj/GeantReweight/Sim/${JOBSUBJOBID:0:10}/Ar_"+e+unit+"_nom_inel1_elast1.5_${MAKEUP}.root\n"  
      outfile.write(newline)

      newline = "./G4Reweight -f ${_CONDOR_SCRATCH_DIR}/code/G4Sim-build/Ar_nom_"+e+unit+"_${MAKEUP}.root -o Ar_"+e+unit+"_nom_inel1.5_elast1.5_${MAKEUP}.root -i 1.5 -e 1.5 \n"
      outfile.write(newline)

      newline = "ifdh cp Ar_"+e+unit+"_nom_inel1.5_elast1.5_${MAKEUP}.root /pnfs/dune/scratch/users/calcuttj/GeantReweight/Sim/${JOBSUBJOBID:0:10}/Ar_"+e+unit+"_nom_inel1.5_elast1.5_${MAKEUP}.root\n"  
      outfile.write(newline)
    else:
      newline = "./G4Reweight -f ${_CONDOR_SCRATCH_DIR}/code/G4Sim-build/Ar_"+samp+"_"+e+unit+"_${MAKEUP}.root -o Ar_"+e+unit+"_"+samp+"_${MAKEUP}.root -i 1. -e 1. \n"
      outfile.write(newline)

      newline = "ifdh cp Ar_"+e+unit+"_"+samp+"_${MAKEUP}.root /pnfs/dune/scratch/users/calcuttj/GeantReweight/Sim/${JOBSUBJOBID:0:10}/Ar_"+e+unit+"_"+samp+"_${MAKEUP}.root\n"
      outfile.write(newline)
