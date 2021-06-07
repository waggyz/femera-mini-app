more off; format short; format compact;
graphics_toolkit("gnuplot");
PCT=0.01;
%
lws='linewidth' ; hlw= 0.25; flw= 1.5; lw = 2; mlw=1; tlw=4;
mss='markersize'; sms= 2; mms = 4; bms=5; gms=6;
fss='fontsize'  ; fs =11; bfs=12; tfs=10; lfs=7;
has='horizontalalignment';
vas='verticalalignment';
fas='fontangle';
%
ns=1e-9; us=1e-6; ms=1e-3; sc=1; mn=60; hr=60*mn; dy=24*hr;
%
prep_cncr=nan;
%
figdir   = '../build/tests/'
filebase = '../build/tests/sizes',
%filebase = '../build/tests/xs-md-10fmt',
%
%filebase ='../data/perf/mini.0.2-baseline/sizes', prep_cncr=1,
%figdir   = '../data/perf/mini.0.2-baseline/'
%
%
csv = dlmread ('perf/iters2solve-1e-5.csv');
%
if (0==1);
  figure (99); clf; hold on; grid on;
  loglog (csv(:,1),csv(:,2),'-bo',lws,3);
  f = 5:numel(csv(:,1));
  poly = polyfit (log(csv(f,1)),log(csv(f,2)), 1),
  loglog (csv(:,1),exp(polyval(poly,log(csv(:,1)))),'-r',lws,1);
  %poly3 = poly; poly3(1)=1/3; poly3,
  poly3 = [1/3,1.5],
  loglog (csv(:,1),exp(polyval(poly3,log(csv(:,1)))),'-m',lws,1);
  %loglog (csv(:,1),exp( log (csv(:,1))*1/3+1.5 ),'-k',lws,1);
  return
end;
%
csv = dlmread ([filebase,'.csv']);
%csv = dlmread ('../build/tests/sizes-no-solve.csv');
%csv = dlmread ('../build/tests/sizes-sleep.csv');
%
mesh_time = dlmread ([filebase,'-time.csv']);
ram_total_GB = 193637764 * 1024 / 1e9,%FIXME xs-md skylake
%
start = min(csv(:,6));
stop  = max(csv(:,7));
%
elapsed = stop-start;
if (elapsed*ns < 360);
  xunit = 1/sc; xunitstr='s';
else;
  xunit = 1/mn; xunitstr='m';
end;
%
figure (100); clf; hold on; grid on;
axis ([0,elapsed*ns*xunit, -1.0,3.5]);
xlabel (['Elapsed time (',xunitstr,')']);
ylabel ('Physical CPU');
set (gca,'ytick',[0:3]);
barw=20;
%
chck = find (csv(:,5)==1);
scan = find (csv(:,5)==1.1);
info = find (csv(:,5)==1.2);
mesh = find (csv(:,5)==2);
prep = find (csv(:,5)==3);
iter = find (csv(:,5)==4);
zzzz = find (csv(:,5)==5);
post = find (csv(:,5)==10);
%
x = nan(3*numel(chck),1); y=x;
x(1:3:end) = (csv(chck,6)-start)*ns*xunit; y(1:3:end) = csv(chck,1);
x(2:3:end) = (csv(chck,7)-start)*ns*xunit; y(2:3:end) = csv(chck,1);
plot (x,y,'-k',lws,barw);
%
x = nan(3*numel(scan),1); y=x;
x(1:3:end) = (csv(scan,6)-start)*ns*xunit; y(1:3:end) = csv(scan,1);
x(2:3:end) = (csv(scan,7)-start)*ns*xunit; y(2:3:end) = csv(scan,1);
plot (x,y,'-y',lws,barw);
%
%x = nan(3*numel(info),1); y=x;
%x(1:3:end) = (csv(info,6)-start)*ns*xunit; y(1:3:end) = csv(info,1);
%x(2:3:end) = (csv(info,7)-start)*ns*xunit; y(2:3:end) = csv(info,1);
%plot (x,y,'-c',lws,barw);
%
x = nan(3*numel(mesh),1); y=x;
x(1:3:end) = (csv(mesh,6)-start)*ns*xunit; y(1:3:end) = csv(mesh,1);
x(2:3:end) = (csv(mesh,7)-start)*ns*xunit; y(2:3:end) = csv(mesh,1);
plot (x,y,'-m',lws,barw);
%
x = nan(3*numel(prep),1); y=x;
x(1:3:end) = (csv(prep,6)-start)*ns*xunit; y(1:3:end) = csv(prep,1);
x(2:3:end) = (csv(prep,7)-start)*ns*xunit; y(2:3:end) = csv(prep,1);
plot (x,y,'-b',lws,barw);
%
if (numel(iter)); solv_text='No Solve'; solvshort='no-solve';
  x = nan(3*numel(iter),1); y=x;
  x(1:3:end) = (csv(iter,6)-start)*ns*xunit; y(1:3:end) = csv(iter,1);
  x(2:3:end) = (csv(iter,7)-start)*ns*xunit; y(2:3:end) = csv(iter,1);
  plot (x,y,'-w',lws,barw);
end;
if (numel(zzzz)); solv_text='Sleep'; solvshort='sleep';
  x = nan(3*numel(zzzz),1); y=x;
  x(1:3:end) = (csv(zzzz,6)-start)*ns*xunit; y(1:3:end) = csv(zzzz,1);
  x(2:3:end) = (csv(zzzz,7)-start)*ns*xunit; y(2:3:end) = csv(zzzz,1);
  plot (x,y,'-r',lws,barw);
end;
%
x = nan(3*numel(post),1); y=x;
x(1:3:end) = (csv(post,6)-start)*ns*xunit; y(1:3:end) = csv(post,1);
x(2:3:end) = (csv(post,7)-start)*ns*xunit; y(2:3:end) = csv(post,1);
plot (x,y,'-c',lws,barw);
%
plot ((csv(:,6)-start)*ns*xunit,csv(:,1),'>g');
plot ((csv(:,7)-start)*ns*xunit,csv(:,1),'or');
%
legend (...
  'Check','Scan','Mesh','Prep',solv_text,'Post',...
  'Begin','Done',...
  'location','south', 'orientation','horizontal');
%
figure (100); hold on;
paper=[0.25,0.25, 6,3];
set(gcf,'paperposition',paper);
figname=['sizes-gandt-',solvshort],
print([figdir,figname,'.eps'],'-depsc2','-FHelvetica');
print([figdir,figname,'.pdf'],'-depsc2','-FHelvetica');
%
%==============================================================================
%encolor = ['b';'k';'m'];
sim_n = size (mesh_time, 1),
sim_n = numel(unique(csv(csv(:,2)>0,2))),
sim_n = max (csv(:,2)),
tmin = 0;%0.100 *cncr/sim_n *0,%FIXME 100ms startup time, pro-rated
%
dofs_span = 10.^((3.0-1e-9):0.1:10);
f=find(csv(:,10)>0);
dofs_min=min(csv(f,10)), dofs_avg=mean(csv(f,10)), dofs_max=max(csv(f,10)),
%
ref_cncr = 40,
run_cncr = max(csv(:,1))+1,
if (isnan(prep_cncr)); prep_cncr=run_cncr, end;
%ref_cncr = run_cncr;%TODO Remove ref_cncr?
mesh_time(:,1) = mesh_time(:,1) / prep_cncr;
%
% Estimates of solver mem use from mini-app v0.1
lof_dof=25932897; lof_byte=2705536 *1000; byte_per_dof = lof_byte/lof_dof,
%
run_time = (max(csv(:,7)) - min(csv(:,6)))*ns,
prep_time = sum(mesh_time(:,1)),
tot_time = run_time + prep_time,
avg_run_time = run_time / sim_n,
avg_tot_time = tot_time / sim_n,
%
grgb = [0,0,10]/16;% [0,5/8,0];
yrgb = [16,8,0]/16; crgb = [0,10,10]/16; cos='color';
%
fig_n = 3,
for (fig = [1:fig_n]); figure (fig); clf; hold on; grid on;
  %
  set (gca, 'position',  [0.10,0.10, 0.80, 0.80]);
  set (gca, fss,lfs);
  if (1==1);%(run_cncr == ref_cncr);
    s='performance';
  else;
    s='estimate';
  end;
  title (['Baseline batch ',s,' per Skylake node, 40 sims/40 cpu'], fss,tfs);
  ax = [1000,10e9, 10*us,3*dy];
  axis (ax);
  %
  text(50e9,exp ((log(ax(3))+log(ax(4)))*0.5),...
    '\leftarrow Lower is better. (time/sim/node)',...
    has,'center',vas,'top','rotation',90,fss,lfs);
  %
  if (1==0)
    text (3e10,1,'Time','rotation',90,...
      'verticalalignment','top','horizontalalignment','center');
    y=10.^(-4:5);
    x=1.23e10*ones(size(y));
    ytic = ['10^{-4}';'10^{-3}';'10^{-2}';'10^{-1}';...
            '10^0   ';'10^1   ';'10^2   ';'10^3   ';'10^4   ';'10^5   '];
  else;
    y=[100*us,1*ms,10*ms,100*ms,1*sc,6*sc,1*mn,6*mn,1*hr,1*dy,3*dy];
    x=1.23e10*ones(size(y));
    ytic = ['100 \mus';'1 ms';'10 ms';'100 ms';...
            '1 s   ';'6 s   ';'1 m   ';'6 m   ';'1 h   ';'1 d   ';'3 d   '];
  end;
  text (x,y, ytic, fss,lfs);
  %
  xlabel ('3D linear-elastic isotropic simulation size (dof)');
  if (run_cncr == ref_cncr);
    s='Expected';
  else;
    s='Estimated';
  end;
  ylabel ([s,' throughput (sims/time/node)']);
  %
  set (gca,'xtick', 10.^[1e-9:11]);% hack around integer overflow for xtick 10^10
  ytic = 1./[1/(3*dy),1/dy,1/hr,10/hr,1/mn,10/mn,1/sc,...
    10/sc,100/sc,1e3/sc,10e3/sc];
  set (gca,'ytick', ytic);
  ylab = ['1/3 d';'1/d';'1/h';'10/h';'1/m';...
         '10/m';'1/s';'10/s';'100/s';'1k/s';'10k/s'];
  set (gca,'yticklabel',ylab);
  %
  fr = sc/24; loglog ([1e3,1e10],[fr,fr],'--b', lws,hlw, mss,ms);
  text (1e10,fr,'24 sims/s/node (Movie)','color','b',...
    has,'right',vas,'bottom', fss,lfs);
  fr = sc/30; loglog ([1e3,1e10],[fr,fr],'--b', lws,hlw, mss,ms);
  text (1e10,fr,'30 sims/s/node (NTSC)','color','b',...
    has,'right',vas,'top', fss,lfs);
  %
  x40=exp(sum(log([1e6,1e7]))*0.5);
  mx40 = 1./(40/(3*dy));
  text (x40,mx40,'40 sims/3 d/node','color','b',
    has,'center', vas,'bottom', fss,lfs);
  %
  mem = 96; mx = mem*1e9 / byte_per_dof /40;
  loglog ([mx,mx],[1e-5,3*dy],':b', lws,hlw);
  text (mx, 15e-6, [num2str(mem),' GB/40 sims/node'],...
    'rotation',90, has,'left', vas,'bottom', 'fontsize',lfs, 'color','b');
  mem =375; mx = mem*1e9 / byte_per_dof /40;
  loglog ([mx,mx],[1e-5,3*dy],'-.b', lws,hlw);
  text (mx, 15e-6, [num2str(mem),' GB/40 sims/node'],
    'rotation',90, has,'left', vas,'bottom', 'fontsize',lfs, 'color','b');
  mem =96; mx = mem*1e9 / byte_per_dof /1;
  loglog ([mx,mx],[1e-5,3*dy],'--b', lws,hlw);
  text (mx, 15e-6, [num2str(mem),' GB/sim/node'],
    'rotation',90, has,'left', vas,'bottom', 'fontsize',lfs, 'color','b');
  mem =375; mx = mem*1e9 / byte_per_dof /1;
  loglog ([mx,mx],[1e-5,3*dy],'-b', lws,hlw);
  text (mx, 15e-6, [num2str(mem),' GB/sim/node'],
    'rotation',90, has,'left', vas,'bottom', 'fontsize',lfs, 'color','b');
  %
  szstr = ['XS';'SM';'MD';'LG';'XL'];
  szpos = [e*1e3;1e5;e*1e6;1e8;e*1e9]; y = 3*dy *ones(size(szpos));
  text (szpos,y, szstr, has,'center', vas,'bottom', 'fontsize',lfs, 'color','b');
  loglog ([1e3,10e9],[3*dy,3*dy], 'b', lws,lw);
  sz = [10e3;1e6;10e6;1e9];
  x=nan(numel(sz)*3,1); x(1:3:end)=sz; x(2:3:end)=sz;
  y=nan(size(x)); y(1:3:end)=2*dy; y(2:3:end)=3*dy;
  loglog (x,y, 'b', lws,3);
end;% fig loop
%
iters_poly = [1/3,1.5];
speed = 1e9 - (dofs_span>10e3).* 90e6.*(log(dofs_span)-log(10e3));
speed (speed < 500e6) = 500e6;
iters = exp (polyval (iters_poly, log (dofs_span)));
solv_secs = iters .* dofs_span ./ speed;% avg time to solve 1 sim (using 40 cores)
%
solv_text_dofs=1.5e8;
speed = 1e9 - (solv_text_dofs>10e3).* 90e6.*(log(solv_text_dofs)-log(10e3));
speed (speed < 500e6) = 500e6;
iters = exp (polyval (iters_poly, log (solv_text_dofs)));
solv_text_secs = iters .* solv_text_dofs ./ speed;
solv_text = ['max |u_{ij}^{FEM}-u_{ij}^{exact}| <10^{-5}'];
%
init=prep;
prep = mesh_time(:,1) * run_cncr / ref_cncr;
si = csv(:,2);
chck = mesh = read = solv = dofs = chck = zeros (sim_n,1);
f = find (csv(:,10)>0); dofs(si(f)) = csv(f,10);
f = find (csv(:,5)==1); chck(si(f)) = tmin + (csv(f,7)-csv(f,6))*ns / ref_cncr;
f = find (csv(:,5)==2); mesh(si(f)) = tmin + (csv(f,7)-csv(f,6))*ns / ref_cncr;
f = find (csv(:,5)==3); read(si(f)) = tmin + (csv(f,7)-csv(f,6))*ns / ref_cncr;
f = find (csv(:,5)==5); solv(si(f)) = tmin + (csv(f,7)-csv(f,6))*ns / ref_cncr;
simt = prep+chck+mesh+read+solv;
prep(prep<=0)=nan;
chck(chck<=0)=nan;
mesh(mesh<=0)=nan;
read(read<=0)=nan;
solv(solv<=0)=nan;
%
if (0);
  fchck = find (csv(:,5)==1);
  fsolv = find (csv(:,5)==5);
  chck_start = nan(size(prep));
  solv_done = nan(size(prep));
  chck_start(csv(fchck,2)) = csv(fchck,6);
  solv_done (csv(fsolv,2)) = csv(fsolv,7);
  runt = prep + (solv_done-chck_start)*ns / ref_cncr;
  lost_time_ratio = (simt - runt)./runt;
  time_lost_pct...
    = 100* [min(lost_time_ratio), mean(lost_time_ratio), max(lost_time_ratio)],
end;
w=ones(size(dofs));
%w(dofs>0)=1./log(dofs(dofs>0));% Fitting weights.

wdofs=dofs.*w; wprep=prep.*w; wchck=chck.*w; wmesh=mesh.*w; wread=read.*w;
wsolv=solv.*w;
%
f=find(mesh>0);%(geo,geo_unrolled)
poly_mesh_cads = polyfit (wdofs(f),wmesh(f),1),
poly_mesh_cads(end) = min(wmesh(f)),
poly_read_cads= polyfit (wdofs(f),wread(f),1),
poly_read_cads(end) = min(wread(f)),
%
fread = unique(csv(find(csv(:,4)~=5),2)); fread(fread<=0)=[];% NOT cgns
poly_read=nan;
if(numel(fread)>0);
  poly_read = polyfit (wdofs(fread),wread(fread),1),
  if (poly_read(end)<0);% Assume y-intercept is min(wread(fread).
    poly_read(end)=min(wread(fread)),
  end;
end;
fstls = unique( csv( find( csv(:,4)==3),2));% stl
fcgns = unique( csv( find( csv(:,4)==5),2));% cgns
poly_read_cgns=nan;
poly_chck_cgns=nan;
if(numel(fcgns)>0);
  poly_read_cgns = polyfit (wdofs(fcgns),wread(fcgns),1),
  poly_chck_cgns = polyfit (wdofs(fcgns),wchck(fcgns),0),
  %poly_chck_cgns(end) = min(wchck(fcgns)),
end;
%
fmesh = unique( csv( find( csv(:,4)~=1),2));% NOT CAD (geo,geo_unrolled)
poly_prep_mesh=nan;
if(numel(fmesh)>0);
  fmesh(fmesh<=0)=[]; fmesh(prep(fmesh)<=0)=[];
  poly_prep_mesh = polyfit (wdofs(fmesh),wprep(fmesh),1),
  %poly_prep_mesh(end) = min(wprep(fmesh)),
end;
fgmsh = unique( csv( find( csv(:,4)~=5),2));% NOT CGNS
fgmsh(fgmsh<=0)=[];
poly_read_gmsh = polyfit (wdofs(fgmsh),wread(fgmsh),1),
if(numel(wread)>0); if (poly_read_gmsh(end)<0);
  poly_read_gmsh(end)=min(wread(fgmsh)),
end; end;
%
fstls = unique( csv( find( csv(:,4)==3),2));% STL (surface mesh)
fstls(fstls<=0)=[];
if (0);% Not linear!
  poly_prep_stls = polyfit (wdofs(fstls),wprep(fstls),1),
  if(numel(wprep)>0); if (poly_prep_stls(end)<0);
    poly_prep_stls(end)=min(wprep(fstls)),
  end; end;
else;
    po32_prep_stls = polyfit(dofs(fstls), prep(fstls).^(3/2)-min(wprep(fstls)).^(3/2),1);
    po32_prep_stls(end)=min(wprep(fstls)).^(3/2),
end;
%
fothr = unique( csv( find(...
  (csv(:,4)~=1) & (csv(:,4)~=5) & (csv(:,4)~=3)),2));%NOT CAD, NOT CGNS, NOT STL
fothr(fothr<=0)=[]; fothr(prep(fothr)<=0)=[];
poly_chck=nan;
if (numel(fothr)>0);
  poly_chck = polyfit (wdofs(fothr),wchck(fothr),1);
  if (poly_chck(end)<0); poly_chck(end)=min(wchck(fothr)), end;
end;
fmshb = unique(csv(find(csv(:,3)==1 & csv(:,4)==4),2));% msh4 binary
poly_chck_msh4=nan;
if (numel(fmshb)>0);
  poly_chck_msh4 = polyfit (wdofs(fmshb),wchck(fmshb),1),
  if (poly_chck_msh4(end)<0);% Re-fit, assuming y-intercept is min.
    poly_chck_msh4(end)=min(wchck(fmshb)),
  end;
end;
%poly_read_mshb = polyfit (wdofs(fmshb),wread(fmshb),1),
%if (poly_read_mshb(end)<0);% Re-fit, assuming y-intercept is min.
%  rmin = min(wread(fmshb));
%  poly_read_mshb = polyfit (wdofs(fmshb),(wread(fmshb)-rmin)./wdofs(fmshb),0);
%  poly_read_mshb = [poly_read_mshb,cmin],
%end;
%
fcads = unique(csv(find(csv(:,4)==1),2));% CAD (geo,geo_unrolled)
if (0)
  [cads_near_1Mdof,icads_1Mdof] = min(abs(dofs(fcads)-1e6)),
  dofs_cads_1Mdof = dofs(fcads(icads_1Mdof)),
  simt_cads_1Mdof = simt(fcads(icads_1Mdof)),
  %
  [mesh_near_1Mdof,imesh_1Mdof] = min(abs(dofs(fmesh)-1e6)),
  dofs_mesh_1Mdof = dofs(fmesh(imesh_1Mdof)),
  simt_mesh_1Mdof = simt(fmesh(imesh_1Mdof)),
end;
cnst_prep_cads_gmsh=nan;
cnst_prep_cads_bash=nan;
cnst_chck_cads_geos=nan;
if(numel(fcads)>0);
  fcads(fcads<=0)=[]; fcads(wchck(fcads)<=0)=[];
  %avg = mean(prep(fcads));
  %fgeos = fcads(prep(fcads)<avg/2); fgeou=fcads(prep(fcads)>avg/2);
  fgeou = unique(csv(find(csv(:,3)==2 & csv(:,4)==1),2));
  fgeos = unique(csv(find(csv(:,3)==3 & csv(:,4)==1),2));
  fgeou(fgeou<=0)=[]; fgeou(wprep(fgeou)<=0)=[];
  fgeou(fgeos<=0)=[]; fgeou(wprep(fgeos)<=0)=[];
  cnst_prep_cads_gmsh = polyfit (wdofs(fgeou),wprep(fgeou),0),% geo_unrolled
  cnst_prep_cads_bash = polyfit (wdofs(fgeos),wprep(fgeos),0),% geo
  cnst_chck_cads_geos = polyfit (wdofs(fcads),wchck(fcads),0),
end;
%
femera_cgns_data_speedup_xsmd(1) = (poly_chck(end)+poly_read_gmsh(end))...
  / (poly_read_cgns(end)+poly_chck_cgns(end));
femera_cgns_data_speedup_xsmd(2)= (poly_chck(1)+poly_read_gmsh(1))...
  / poly_read_cgns(1);
femera_cgns_data_speedup_xsmd,
%
femera_cads_mesh_speedup_xsmd(1) = poly_prep_mesh(end)...
  / (poly_mesh_cads(end)+cnst_chck_cads_geos);
femera_cads_mesh_speedup_xsmd(2) = poly_prep_mesh(1) / poly_mesh_cads(1);
femera_cads_mesh_speedup_xsmd,
%
for (fig = [1:fig_n]); figure (fig);
%
  xmin=max ([min(dofs(fcgns)),min(dofs(fgeou))]);
  xmax=min ([max(dofs(fcgns)),max(dofs(fgeou))]);
  x=exp(log(xmin):0.1:log(xmax));
  x(x<1e3)=[];
  xy1=sortrows ([dofs(fcgns), simt(fcgns)]);
  y1=interp1(xy1(:,1),xy1(:,2),x);
  %xy2=sortrows ([dofs(fgeou), simt(fgeou)]);
  %y2=interp1(xy2(:,1),xy2(:,2),x);
  iters_poly = [1/3,1.5];
  speed = 1e9 - (dofs_span>10e3).* 90e6.*(log(dofs_span)-log(10e3));
  speed (speed < 500e6) = 500e6;
  iters = exp (polyval (iters_poly, log (dofs_span)));
  secs = iters .* dofs_span ./ speed;
  y2= cnst_prep_cads_gmsh...      % prep
    + cnst_chck_cads_geos...      % chck
    + polyval(poly_mesh_cads,x)...% mesh
    + polyval(poly_read_gmsh,x)...% read
    + interp1(dofs_span,secs,x);  % solv
  %y2(y2<poly_prep_mesh(end)) = poly_prep_mesh(end);
  %y2(y2<cnst_prep_cads_gmsh) = cnst_prep_cads_gmsh;
  frgb=[1.0,1.0,0.5];% yellow
  px = [x(1:(end-1));x(2:end);x(1:(end-1))];
  py = [y1(1:(end-1));y1(2:end);y2(1:(end-1))];
  patch (px,py,'y','facecolor',frgb,'edgecolor','none');
  px = [x(1:(end-1));x(2:end);x(2:(end))];
  py = [y2(1:(end-1));y2(2:end);y1(2:(end))];
  patch (px,py,'y','facecolor',frgb,'edgecolor','none');
  %
  xmin=min(dofs(fgeos));
  xmax=max(dofs(fgeos));
  x=exp(log(xmin):0.1:log(xmax));
%  [~,u,~] = unique(dofs(fgeos));
  xy1=sortrows ([dofs(fgeos), simt(fgeos)]);
  %xy2=sortrows ([dofs(fgeos), simt(fgeos)-prep(fgeos)]);
  y1=interp1(xy1(:,1),xy1(:,2),x);
  %y2=interp1(xy2(:,1),xy2(:,2),x);
  iters_poly = [1/3,1.5];
  speed = 1e9 - (dofs_span>10e3).* 90e6.*(log(dofs_span)-log(10e3));
  speed (speed < 500e6) = 500e6;
  iters = exp (polyval (iters_poly, log (dofs_span)));
  secs = iters .* dofs_span ./ speed;
  y2= cnst_chck_cads_geos...      % chck
    + polyval(poly_mesh_cads,x)...% mesh
    + polyval(poly_read_gmsh,x)...% read
    + interp1(dofs_span,secs,x);  % solv
  px = [x(1:(end-1));x(2:end);x(1:(end-1))];
  py = [y1(1:(end-1));y1(2:end);y2(1:(end-1))];
  frgb=[0.5,1.0,0.5];% green
  patch (px,py,'g','facecolor',frgb,'edgecolor','none');
  px = [x(1:(end-1));x(2:end);x(2:(end))];
  py = [y2(1:(end-1));y2(2:end);y1(2:(end))];
  patch (px,py,'g','facecolor',frgb,'edgecolor','none');
  %
  x=dofs_span; x(end)=x(end)*0.95;
  y1=polyval(poly_read_cgns,x);
  %y2=polyval(poly_read_cads,x);
  y2=polyval(poly_read_gmsh,x);
  l=find (y1<y2);
  x=x(l); y1=y1(l); y2=y2(l);
  px = [x(1:(end-1));x(2:end);x(1:(end-1))];
  py = [y1(1:(end-1));y1(2:end);y2(1:(end-1))];
  frgb=[1,1,1]*0.9;% gray
  patch (px,py,'k','facecolor',frgb, 'edgecolor','none');
  px = [x(1:(end-1));x(2:end);x(2:(end))];
  py = [y2(1:(end-1));y2(2:end);y1(2:(end))];
  patch (px,py,'k','facecolor',frgb, 'edgecolor','none');
  %
  switch (fig);
  case (1);% fig
    loglog ([120e3,10e9],[mx40,mx40],'-b', lws,flw);
    loglog (dofs(fmesh), prep(fmesh),'<c',cos,crgb,'markersize',sms,lws,mlw);
    loglog (dofs(fgeou), prep(fgeou),'<m','markersize',sms,lws,mlw);
    loglog (dofs(fgeos), prep(fgeos),'<r','markersize',sms,lws,mlw);
    %
    loglog (dofs(fothr), chck(fothr),'+c',cos,crgb,'markersize',sms,lws,mlw);
    loglog (dofs(fcads), chck(fcads),'+r','markersize',sms,lws,mlw);
    loglog (dofs(fcgns), chck(fcgns),'+k','markersize',sms,lws,mlw);
    loglog (dofs(fmshb), chck(fmshb),'sy',cos,yrgb,'markersize',sms,lws,mlw);
    %
    loglog (dofs, mesh,'^r','markersize',sms+1,lws,mlw);
    %
    loglog (dofs(fgmsh), read(fgmsh),'or','markersize',sms,lws,mlw);
    %loglog (dofs(fcads), read(fcads),'or','markersize',sms,lws,mlw);
    loglog (dofs(fcgns), read(fcgns),'ok','markersize',sms,lws,mlw);
    %loglog (dofs(fmshb), read(fmshb),'xy',cos,yrgb,'markersize',sms,lws,mlw);
    %
    loglog (dofs, solv,'*b','markersize',sms-1,lws,mlw);
    loglog (dofs(fstls), chck(fstls),'xg','markersize',sms,lws,mlw);
    loglog (dofs(fstls), prep(fstls),'xg','markersize',sms,lws,mlw);
    loglog (dofs(fstls), mesh(fstls),'xg','markersize',sms,lws,mlw);
  case (2);% fig
    loglog ([120e3,10e9],[mx40,mx40],'-b', lws,flw);
  case (3);% fig
    loglog ([300e3,10e9],[mx40,mx40],'-b', lws,flw);
    %
    mesh(isnan(mesh))=0;
    file = prep+chck+mesh+read;
    %
    loglog (dofs_span,solv_secs,'-b;Solve (estimate);','color',grgb,lws,tlw);
    %
    loglog (dofs(fothr), simt(fothr),'.k;3D flat file total;', mss,mms,lws,mlw);
    loglog (dofs(fcgns), simt(fcgns),'.c;3D CGNS total;', mss,mms,lws,mlw);
    loglog (dofs(fstls), simt(fstls),'.g;2D STL total;', mss,mms,lws,mlw);
    loglog (dofs(fgeou), simt(fgeou),'.m;CAD unrolled total;', mss,mms,lws,mlw);
    loglog (dofs(fgeos), simt(fgeos),'.r;CAD scipt total;', mss,mms,lws,mlw);
    %
    poly = polyfit (dofs(fothr), file(fothr),1); poly(end) = min(prep(fothr));
    loglog (dofs_span, polyval(poly,dofs_span),'-k;3D flat file mesh+I/O;',lws,mlw);
    poly = polyfit (dofs(fcgns), file(fcgns),1); poly(end) = min(prep(fcgns));
    loglog (dofs_span, polyval(poly,dofs_span),'-c;3D CGNS mesh+I/O;',lws,mlw);
    if (0);% not linear!
    poly = polyfit (dofs(fstls), file(fstls),1); poly(end) = min(prep(fstls));
    loglog (dofs_span, polyval(poly,dofs_span),'-g;2D STL mesh+I/O;',lws,mlw);
    else;
    poly_prep = polyfit(dofs(fstls), prep(fstls).^(3/2),1);
    poly_prep(end) = min(prep(fstls).^(3/2));
    poly=polyfit (dofs(fstls), file(fstls)-prep(fstls),1);
    poly(end) = min(prep(fstls)-prep(fstls));
    plot_y = polyval(poly_prep,dofs_span).^(2/3) + polyval(poly,dofs_span);
    loglog (dofs_span,plot_y ,'-g;2D STL mesh+I/O;','markersize',4,lws,lw);
    end;
    poly = polyfit (dofs(fgeou), file(fgeou),1); poly(end) = min(prep(fgeou));
    loglog (dofs_span, polyval(poly,dofs_span),'-m;CAD unrolled mesh+I/O;',lws,mlw);
    poly = polyfit (dofs(fgeos), file(fgeos),1); poly(end) = min(prep(fgeos));
    loglog (dofs_span, polyval(poly,dofs_span),'-r;CAD scripted mesh+I/O;',lws,mlw);
    if (0)
    loglog (dofs(fothr), file(fothr),'+k','markersize',sms,lws,mlw);
    loglog (dofs(fcgns), file(fcgns),'+c','markersize',sms,lws,mlw);
    loglog (dofs(fstls), file(fstls),'+g','markersize',sms,lws,mlw);
    loglog (dofs(fgeou), file(fgeou),'+m','markersize',sms,lws,mlw);
    loglog (dofs(fgeos), file(fgeos),'+r','markersize',sms,lws,mlw);
    end;
    if (0)
    loglog (dofs(fothr), prep(fothr),'<k',cos,crgb,'markersize',sms,lws,mlw);
    loglog (dofs(fcgns), prep(fcgns),'<c','markersize',sms,lws,mlw);
    loglog (dofs(fstls), prep(fstls),'<g','markersize',sms,lws,mlw);
    loglog (dofs(fgeou), prep(fgeou),'<m','markersize',sms,lws,mlw);
    loglog (dofs(fgeos), prep(fgeos),'<r','markersize',sms,lws,mlw);
    loglog (dofs, solv,'*b','markersize',sms-1,lws,mlw);
    end;
    %
    loglog (dofs_span, polyval(poly_read_gmsh,dofs_span),'--k;Gmsh read memory;',lws,lw);
    loglog (dofs_span, polyval(poly_read_cgns,dofs_span),'--c;CGNS read file;',lws,mlw);
    %
  end;% switch (fig)
  if (any (fig == [1,2]));
    loglog (dofs(fmesh), simt(fmesh),'.k;Total saved mesh;','markersize',mms,lws,mlw);
    loglog (dofs_span, polyval(poly_prep_mesh,dofs_span),'-.k;Prep save mesh;',lws,lw);
    loglog (dofs_span, polyval(poly_chck_cgns,dofs_span),':k;Check CGNS;',lws,mlw);
    loglog (dofs_span, polyval(poly_read_cgns,dofs_span),'--k;Read file CGNS;',lws,mlw);
    %loglog (dofs(fcgns), simt(fcgns),'.k;Total CGNS;','markersize',mms,lws,mlw);
    loglog (dofs_span, polyval(cnst_prep_cads_gmsh,dofs_span),'-m;Prep unroll CADs;',lws,mlw);
    loglog (dofs_span, polyval(cnst_prep_cads_bash,dofs_span),'-r;Prep script CADs;',lws,mlw);
    %
    loglog (dofs_span, polyval(cnst_chck_cads_geos,dofs_span),':r;Check CADs;',lws,mlw);
    %
    loglog (dofs_span, polyval(poly_mesh_cads,dofs_span),'-.r;Mesh CADs;',lws,lw);
    %loglog (dofs_span, polyval(poly_read_cads,dofs_span),'--r;Read memory CADs;',lws,mlw);
      loglog (dofs_span, polyval(poly_read_gmsh,dofs_span),'--r;Read memory Gmsh;',lws,lw);
    %
    loglog (dofs(fgeou), simt(fgeou),'.m;Total unroll CADs;','markersize',mms,lws,mlw);
    loglog (dofs(fgeos), simt(fgeos),'.r;Total script CADs;','markersize',mms,lws,lw);
    %
    if (1);
      loglog (dofs_span, polyval(poly_chck     ,dofs_span),':c;Check mesh Gmsh;',cos,crgb,lws,mlw);
    %  loglog (dofs_span, polyval(poly_chck_msh4,dofs_span),'--y;Check msh4 bin;',cos,yrgb,lws,lw);
    %  loglog (dofs(fothr), simt(fothr),'.c;Total other;',cos,crgb,'markersize',mms,lws,mlw);
    end;
    %TODO plot post times (Gmsh vs. CGNS)?
    if(0);% not linear
    loglog (dofs_span, polyval(poly_prep_stls,dofs_span),'-g;Prep STL;',lws,mlw);
    else;
    loglog (dofs_span, polyval(po32_prep_stls,dofs_span).^(2/3),'-g;Prep STL;',lws,mlw);
    end;
    loglog (dofs(fstls), simt(fstls),'.g;Total STL;','markersize',mms,lws,lw);
    %
    %loglog (dofs_span, polyval(poly_read     ,dofs_span),'-k',lws,hlw);
    %loglog (dofs_span, polyval(poly_read_mshb,dofs_span),'-k',lws,hlw);
    loglog (dofs_span,solv_secs,'-b;Solve (estimate);','color',grgb,lws,lw);
  end;% fig < 3
  %
  text (solv_text_dofs,1.2*solv_text_secs,solv_text, 'color',grgb,
    'rotation', 1.7*atand(iters_poly(1)), 'fontsize',lfs, vas,'bottom');
  %
  legend ('location','northwest');
end;% fig loop
figdir='';
figure (1); hold on;
paper=[0.25,0.25, 6,4];
set(gcf,'paperposition',paper);
figname=[filebase,'-samples-',solvshort],
print([figdir,figname,'.eps'],'-depsc2','-FHelvetica');
%print([figdir,figname,'.pdf'],'-dpdf','-FHelvetica');
%print([figdir,figname,'.png'],'-dpng','-FHelvetica');
%
figure (2); hold on;
paper=[0.25,0.25, 6,4];
set(gcf,'paperposition',paper);
figname=[filebase,'-simtime-',solvshort],
print([figdir,figname,'.eps'],'-depsc2','-FHelvetica');
%print([figdir,figname,'.pdf'],'-dpdf','-FHelvetica');
%print([figdir,figname,'.png'],'-dpng','-FHelvetica');
%
figure (3); hold on;
paper=[0.25,0.25, 6,4];
set(gcf,'paperposition',paper);
figname=[filebase,'-summary-',solvshort],
print([figdir,figname,'.eps'],'-depsc2','-FHelvetica');
%
