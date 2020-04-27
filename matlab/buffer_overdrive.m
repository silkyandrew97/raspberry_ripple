%Read Test File
[in,FS] = audioread('110.wav', 'native');
LENGTH = length(in);
%Interface Parameters
FS = single(FS);
N_FRAMES = 64;
%User Parameters
DRIVE = single(0.5);
WINDOW_T = single(0.5);
GAIN_DB = single(0.0);
%Algorithmic Parameters
out = single(zeros(LENGTH,1));
th = single(0.3333333);
buffer_count = 1;
peak = single(0);
peak_count = 1;
%Apply Overdrive
out = drive(in,out,LENGTH,FS,N_FRAMES,DRIVE,WINDOW_T,GAIN_DB,th,buffer_count,peak,peak_count);
%Write To File
audiowrite('119n.wav',out,FS,'BitsPerSample',32);

function out = drive(in,out,LENGTH,FS,N_FRAMES,DRIVE,WINDOW_T,GAIN_DB,th,buffer_count,peak,peak_count)
    %Parameter Initialisation
    gain = single(10^(GAIN_DB/20));
    drive_coeff = 1+(2*(1-DRIVE)^2.5);
    inv_drive_coeff = 1/drive_coeff;
    if (inv_drive_coeff < 2 * th)
        norm_factor = drive_coeff * (3 - (2 - inv_drive_coeff * 3)^2) / 3;
    else
        norm_factor = drive_coeff;
    end
    %Sliding Window Calculations
    %Calculate sliding window size rounded up to nearest block multiple
    window_n = floor(WINDOW_T * FS);
    remainder = mod(window_n,N_FRAMES);
    if(remainder ~= 0)
        window = window_n + N_FRAMES - remainder;
    else
        window = window_n;
    end
    peak_window = window / N_FRAMES;
    window_store = single(zeros(peak_window,1));
    local_store = single(zeros(N_FRAMES,1));
    for i = 1:LENGTH / N_FRAMES
        [peak, window_store, local_store, peak_count, prev_peak] = peak_calcs(in,N_FRAMES,i,buffer_count,window_store,peak_window,peak,local_store,peak_count);
        out(((i-1) * N_FRAMES) + 1:i * N_FRAMES) = effect(in(((i-1) * N_FRAMES) + 1:i * N_FRAMES),N_FRAMES,th,drive_coeff,norm_factor,gain,peak,local_store,prev_peak);
        buffer_count = buffer_count+1;
        peak_count = peak_count + 1;
        if (buffer_count > peak_window)
            buffer_count = 1;
        end
    end
end

function [peak, window_store, local_store, peak_count, prev_peak] = peak_calcs(in,N_FRAMES,tcount,buffer_count,window_store,peak_window,peak,local_store,peak_count)
    prev_peak = peak;
    %Calculate peak from current period
    local_peak = single(0);
    for i = ((tcount - 1) * N_FRAMES) + 1:tcount * N_FRAMES
        absol = abs(in(i));
        if(absol > local_peak)
            local_peak = absol;
        end
    end
    %Assign period peak to window_store
    window_store(buffer_count) = local_peak;
    %If current period peak is larger than what is stored in the window
    if (local_peak > peak)
        %Assign new peak value
        peak = local_peak;
        %Reset peak counter
        peak_count = 1;
        %Peak Smoothing
        j = 1;
        for i=((tcount - 1) * N_FRAMES) + 1:tcount * N_FRAMES
            absol = abs(in(i));
            if (j ~= 1)
                prev_sample = local_store(j-1);
            else
                prev_sample = prev_peak;
            end
            if (absol > prev_sample)
                local_store(j) = absol;
            else
                local_store(j) = prev_sample;
            end
            j = j+1;
        end
    %If largest peak lost from window
    elseif (peak_count > peak_window)
        %Calucate new window peak
        peak = local_peak;
        high = buffer_count;
        for i = buffer_count + 1:peak_window
            if (window_store(i) >= peak)
                peak = window_store(i);
                high = i;
            end
        end
        for i = 1:buffer_count
            if (window_store(i) >= peak)
                peak = window_store(i);
                high = i;
            end
        end
        %Reassign peak counter
        peak_count = buffer_count - high + 1;
        if (high > buffer_count)
            peak_count = peak_count + peak_window;
        end
        %Peak Smoothing
        if (peak < prev_peak)
            linspace = (prev_peak - peak) / N_FRAMES;
            for i=1:N_FRAMES
                local_store(i) = prev_peak - (i * linspace);
            end
        end
    end
end

function out=effect(in,N_FRAMES,th,drive_coeff,norm_factor,gain,peak,local_store,prev_peak)
    out = single(zeros(N_FRAMES,1));
    for i = 1:N_FRAMES
        %Apply Drive Coefficient
        if (peak == prev_peak)
            local_store(i) = peak * drive_coeff;
        else
            local_store(i) = local_store(i) * drive_coeff;
        end
        %Normalise
        norm = in(i) / local_store(i);
        absol = abs(norm);
        %Anomaly Detection
        if ((absol == 0) || (isnan(absol)) || (isinf(absol)))
            out(i) = 0;
        %If No Anomalies Detected
        else
            %Apply Static Characteristic
            if (absol <= th)
                out(i) = 2 * in(i);
            elseif (absol > th) && (absol <= (2 * th))
                if (norm > 0)
                    out(i) = local_store(i) * (3 - (2 - norm * 3)^2) / 3;
                else
                    out(i) = local_store(i) * (-(3 - (2 - (absol * 3))^2) / 3);
                end
            elseif absol > (2 * th)
                if norm > 0
                    out(i) = local_store(i);
                else
                    out(i) = -local_store(i);
                end
            else
                out(i) = in(i);
                disp("[ERROR] in Overdrive Static Characteristic");
            end
            %Drive Coefficent Normalisation
            out(i) = out(i) / norm_factor;
            %Apply Gain
            out(i) = out(i) * gain;
        end
    end 
end