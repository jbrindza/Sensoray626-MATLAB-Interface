% Jordan Brindza
% brindza@seas.upenn.edu
% University of Pennsylvania
% March 2011

function h = encoder_plot()

h.maxDataLen = 300;

h.init = @init;
h.update = @update;

h.init();

  function init()
    % initialize figure
    fig = figure();
    set(fig, 'NumberTitle', 'off'); 
    set(fig, 'name', 'Encoder Test');


    %% initialize subplots

    % encoder ticks
    h.ticks.axes = subplot(2,2,1); cla; 
    title('Encoder Count (Ticks)');
    ylabel('Counts [ticks]');
    xlabel('t');
    set(h.ticks.axes, 'XTickLabel', {});
    axis([0 h.maxDataLen -5000 5000]); 
    hold on;


    % encoder position/angle 
    h.pos.axes = subplot(2,2,2); cla;
    title('Encoder Position (Angle)');
    ylabel('Position [rad]');
    xlabel('t');
    set(h.pos.axes, 'XTickLabel', {});
    axis([0 h.maxDataLen -2*(2*pi) 2*(2*pi)]); 
    hold on;

    % encoder velocity
    h.vel.axes = subplot(2,2,3); cla;
    title('Encoder Velocity');
    ylabel('Velocity [rad/s]');
    xlabel('t');
    set(h.vel.axes, 'XTickLabel', {});
    axis([0 h.maxDataLen -5*(2*pi) 5*(2*pi)]); 
    hold on;

    % encoder acceleration
    h.accel.axes = subplot(2,2,4); cla;
    title('Encoder Acceleration');
    ylabel('Acceleration [rad/s^2]');
    xlabel('t');
    set(h.accel.axes, 'XTickLabel', {});
    axis([0 h.maxDataLen -50*(2*pi) 50*(2*pi)]); 
    hold on;

    %% initialize plots

    h.colors = ['r', 'g', 'b', 'k', 'y', 'm'];

    axes(h.ticks.axes);
    h.ticks.data = zeros(6, h.maxDataLen);
    h.ticks.plots = zeros(6,1);
    for i = 1:6
      h.ticks.plots(i) = plot(h.ticks.data(i,:), h.colors(i));
    end

    axes(h.pos.axes);
    h.pos.data = zeros(6, h.maxDataLen);
    h.pos.plots = zeros(6,1);
    for i = 1:6
      h.pos.plots(i) = plot(h.pos.data(i,:), h.colors(i));
    end

    axes(h.vel.axes);
    h.vel.data = zeros(6, h.maxDataLen);
    h.vel.plots = zeros(6,1);
    for i = 1:6
      h.vel.plots(i) = plot(h.vel.data(i,:), h.colors(i));
    end

    axes(h.accel.axes);
    h.accel.data = zeros(6, h.maxDataLen);
    h.accel.plots = zeros(6,1);
    for i = 1:6
      h.accel.plots(i) = plot(h.accel.data(i,:), h.colors(i));
    end

  end

  function update()
    % get latest counter value
    encoder = s626('encoder')+0;

    % update plots
    h.ticks.data = [h.ticks.data(:,2:end), encoder(:, 1)];
    h.pos.data = [h.pos.data(:,2:end), encoder(:, 2)];
    h.vel.data = [h.vel.data(:,2:end), encoder(:, 3)];
    h.accel.data = [h.accel.data(:,2:end), encoder(:, 4)];
    
    for i = 1:6
      set(h.ticks.plots(i), 'YData', h.ticks.data(i, :));
      set(h.pos.plots(i), 'YData', h.pos.data(i, :));
      set(h.vel.plots(i), 'YData', h.vel.data(i, :));
      set(h.accel.plots(i), 'YData', h.accel.data(i, :));
    end
  end
end

