more off; format short; format compact;
graphics_toolkit("gnuplot");
%
dir_dat='../fig-data/',
csv=[nan];
ratio_tol=2;
switch( 4 );
case( 1 );
  partd=40; part0=40; part1=5120; ratio_tol=2;
  list_parts = (part0:partd:part1);
  csvname = ['slice-',num2str(partd),'-',num2str(part0),'-',num2str(part1),'.csv'],
  csv=unique([...
    dlmread([dir_dat,'slice-40-40-960.csv']);...
    dlmread([dir_dat,'slice-40-40-1600.csv']);...
    dlmread([dir_dat,'slice-1000-40-1600.csv']);...
    dlmread([dir_dat,'slice-large-40.csv']);...
    ],'rows');
  dlmwrite([dir_dat,csvname], csv );
case( 2 );
  p = [ 4,6,8,12,14,16,20,24,28,32,40,56,64]; ratio_tol=1e6;
  list_parts=[];
  for( n=1:16 );
    list_parts=[list_parts,n*p];
  end;
  list_parts=unique(list_parts);
  csvname = ['slice-small.csv'],
case( 3 );
  list_parts=40*2.^(0:10); ratio_tol=1e6;
  csvname = ['slice-40-large.csv'],
case( 4 );
  list_parts=round(1280*2.^((0:50)./10)/40)*40,
  csvname = ['slice-40-1280-40960.csv'],
case( 10 );
  %facts = [2,2,2,2, 3,5],
  facts = [2,2,2,2, 3,5,7];
  %
  facts = [1,facts];
  partd = prod(facts),
  part0=partd; part1=10*partd; ratio_tol=2;
  list_parts = (part0:partd:part1);
  csvname = ['slice-',num2str(part0),'-',num2str(partd),'-',num2str(part1),'.csv'],
end;
%
for( parts = list_parts );
  if(~ismember( parts, csv(:,1) ));
    facts = [1,factor(parts)];
    nfacts = numel(facts);
    ifacts = (1:nfacts);
    clear list_bin_1 list_bin_2 list_bin_3;
    for( k1 = 1:nfacts );
      bin_1 = nchoosek( ifacts, k1 );
      for( ik1 = 1:size(bin_1,1));
        leftover = ifacts(~ismember(ifacts,bin_1(ik1,:)));
        for( k2 = 1:numel(leftover) );
          bin_2 = nchoosek( leftover, k2 );
          for( ik2 = 1:size(bin_2,1) );
            bin_3 = leftover(~ismember(leftover,bin_2(ik2,:)));
            if(numel(bin_3));
              list_bin_1(end+1,:)=ones(1,nfacts);
              list_bin_2(end+1,:)=ones(1,nfacts);
              list_bin_3(end+1,:)=ones(1,nfacts);
              list_bin_1(end,1:numel(bin_1(ik1,:)))=bin_1(ik1,:);
              list_bin_2(end,1:numel(bin_2(ik2,:)))=bin_2(ik2,:);
              list_bin_3(end,1:numel(bin_3))=bin_3;
            end;
          end;
        end;
      end;
    end;
    list_bin_1 = facts(list_bin_1);
    list_bin_2 = facts(list_bin_2);
    list_bin_3 = facts(list_bin_3);
    %
    list_slice=sort([prod(list_bin_1,2),prod(list_bin_2,2),prod(list_bin_3,2)],2);
    [ratio,imin] = min( list_slice(:,3)./ list_slice(:,1) );
    %
    slice = list_slice(imin,:), parts=prod(slice),
    %
    if( ratio < ratio_tol );
      dlmwrite([dir_dat,csvname], [parts,slice], '-append' );
    else;
      dlmwrite([dir_dat,csvname],-[parts,slice], '-append' );
    end;
  end;
end;
%