% script to plot the current encoder values

encPlot = encoder_plot();

while(1)
  % update plot
  encPlot.update();

  % pause to allow graphics to update
  pause(0.01);
end

