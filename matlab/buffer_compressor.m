%Read Test File
[in,FS] = audioread('unaltered_signal.wav', 'native');
LENGTH = length(in);
%Interface Parameters
FS = single(FS);
N_FRAMES = 64;
%User Parameters
RATIO = single(50.0);
KNEE_WIDTH = single(10.0);
THRESHOLD = single(-60.0);
ATTACK_T = single(0.002);
RELEASE_T = single(0.3);
COMPRESSION_DB = single(6.0);
GAIN_DB = single(0.0);
%Algorithmic Parameters
out = single(zeros(LENGTH,1));
gs = single(zeros(2,1));
%Apply Compression
out = compress(in,out,LENGTH,FS,N_FRAMES,RATIO,KNEE_WIDTH,THRESHOLD,ATTACK_T,RELEASE_T,COMPRESSION_DB,GAIN_DB,gs);
%Write To File
audiowrite('compressed_signal.wav',out,FS,'BitsPerSample',32);

function out = compress(in,out,LENGTH,FS,N_FRAMES,RATIO,KNEE_WIDTH,THRESHOLD,ATTACK_T,RELEASE_T,COMPRESSION_DB,GAIN_DB,gs)
    %Parameter Initialisation
    comps = single(10^(COMPRESSION_DB/20) - 1);
    gain = single(10^(GAIN_DB/20));
    if (ATTACK_T == 0)
        att = single(0);
    else
        att = single(exp(-log10(9)/(FS * ATTACK_T)));
    end
    if (RELEASE_T == 0)
        rel = single(0);
    else
        rel = single(exp(-log10(9)/(FS * RELEASE_T)));
    end
    for i=1:LENGTH/N_FRAMES
        %Effect
        [out(((i-1) * N_FRAMES) + 1:i * N_FRAMES), gs] = parallel(in(((i-1) * N_FRAMES) + 1:i * N_FRAMES),N_FRAMES,RATIO,KNEE_WIDTH,THRESHOLD,att,rel,comps,gain,gs);
    end
end

function [out, gs] = parallel(in,N_FRAMES,RATIO,KNEE_WIDTH,THRESHOLD,att,rel,comps,gain,gs)
    out=single(zeros(N_FRAMES,1));
    for i=1:N_FRAMES
        %Anomaly Detection
        absol = abs(in(i));
        if ((absol == 0) || (isnan(absol)) || (isinf(absol)))
            out(i) = 0;
            gc = single(0);
            %Maintain gs continuity when anomaly detected
            if (gc <= gs(1))
                gs(2) = (att * gs(1));
            elseif (gc > gs(1))
                gs(2) = (rel * gs(1));
            else
                gs(2) = (att * gs(1));
            disp("[ERROR] in Anomaly Gain Smoothing");
            end
        %If No Anomalies Detected
        else
            %Convert Input Signal to dB
            db = 20*log10(absol);
            %Gain Computer
            if (db < (THRESHOLD - 0.5 * KNEE_WIDTH))
                sc = db;
            elseif ((db >= (THRESHOLD - 0.5 * KNEE_WIDTH)) && (db < (THRESHOLD + 0.5 * KNEE_WIDTH)))
                sc = db + (((1.0 / RATIO) - 1.0) * (db - THRESHOLD + 0.5 * KNEE_WIDTH)^2) / (2.0 * KNEE_WIDTH); 
            elseif (db >= (THRESHOLD + 0.5 * KNEE_WIDTH))
                sc = THRESHOLD + (db - THRESHOLD) / RATIO;
            else
                sc = db;
                disp("[ERROR] in Gain Computer");
            end
            gc = sc - db;
            %Gain Smoothing
            if (gc <= gs(1))
                gs(2) = (att * gs(1)) + (1.0 - att) * gc;
            elseif (gc > gs(1))
                gs(2) = (rel * gs(1)) + (1.0 - rel) * gc;
            else
                gs(2) = (att * gs(1)) + (1.0 - att) * gc;
                disp("[ERROR] in Gain Smoothing");
            end
            %Apply Linear Gain and Parallelisation
            lin = 10^(gs(2)/20);
            out(i) = (comps * in(i) * lin) + in(i);
            %Apply Gain
            out(i) = out(i) * gain;
        end
        gs(1) = gs(2);
    end
end