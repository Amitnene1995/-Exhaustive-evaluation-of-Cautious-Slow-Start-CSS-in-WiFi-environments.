for i in {100..500..200}
  do 
    ./waf --run "scratch/fm5_xsu --linkbw=$i" --vis
  done
