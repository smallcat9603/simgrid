#!/usr/bin/ruby -Ku
# -*- coding: utf-8 -*-
# 
# 	ラック配置最適化 (lay.rb の後に使う)
# 	Optimize rack layout by length/power/energy. Use after lay.rb
# 	Ikki Fujiwara <ikki@nii.ac.jp>
# 

RACK_WIDTH     =  60 # Rack width [cm]
RACK_DEPTH     = 210 # Rack depth including aisle [cm]


$help_message = <<-EOM
Usage:
  #{$0} [Options] Input_file CableCoeff EnergyCoeff RedundancyCoeff TimeLimit Method

Options:
  -r	reproduce the results from log file; don't run solver
  -f	overwrite existing log file
  -q	be quiet
  --euclidean
    	use diagonal cabling in place of rectilinear (Manhattan) cabling
  --hist
    	output histogram of cable length in addition to a summary
  --traffic 
      search for traffic files
  --seed 
      seed given for the layout program
  --layoutclustering 
      use clustering file produced by the layout script
  --use-patch-panels
  		use patch-panels (instead of direct cabling)
  --cost-norm
  		defines degree for cost norm across stages (1 for manhattan [default], 2 for euclidian)
Input file:
  the configuration file

CableCoeff, EnergyCoeff:
	A numeric value to balance the impact of cable cost or cable power on the search process  

RedundancyCoeff:
	A numeric value for enclosing redundancy requirements in the cable cost. The goal is to 

TimeLimit: 
	The total time limit for computations, in seconds. 

Method:
  sa   => simulated annealing by Taillard
  base => better one of oneway & zigzag
EOM

#' 

require "optparse"
require "time"
require_relative "config"


def checkfile(filename,type)
	if !File.size?(filename)
		STDERR.puts "'#{filename}' of type #{type} cannot be found, maybe not in the same directory?"
		exit(1)
	end
end
	
def main
	$opts = Hash.new
    $opts[:lastyear]=1
	opt = OptionParser.new
	opt.on("-r") do |v|
		$opts[:reproduce] = v
	end
	opt.on("-f") do |v|
		$opts[:overwrite] = v
	end
	opt.on("-q") do |v|
		$opts[:quiet] = v
	end
	opt.on("--layout TYPE") do |v|
		$opts[:layout] = v
	end
	opt.on("--euclidean") do |v|
		$opts[:euclidean] = v
	end
	opt.on("--hist") do |v|
		$opts[:output_histogram] = v
	end
	opt.on("--traffic") do |v|
		$opts[:traffic] = v
	end
	opt.on("--layoutclustering") do |v|
		$opts[:layoutclustering] = v
	end
	opt.on("--seed SEED") do |v|
		$opts[:seed] = v
	end
	opt.on("--use-patch-panels") do |v|
		$opts[:usepatchpanels] = v
	end
	opt.on("--cost-norm DEGREE") do |v|
		$opts[:costnorm] = v
	end
	opt.on("--lastyear LAST_YEAR") do |v|
		$opts[:lastyear] = v
	end
   opt.parse!(ARGV)
	if ! $opts[:seed] 
		$opts[:seed]=0;
	end
	unless ARGV[0] && ARGV[1] && ARGV[2] && ARGV[3]  && ARGV[4] && ARGV[5]
		puts "Not enough arguments!\n"+$help_message
		exit(1)
	end
	if $opts[:reproduce] && $opts[:overwrite]
		STDERR.puts "Options -r and -f cannot be used at the same time."
		exit(1)
	end
	

	config = MyConfig.new(ARGV[0],$opts[:seed])
	
	#===========================================================
	#   Input
	#===========================================================

	layout=nil;
	
	config.topologies.each{ |topo|
	
		STDOUT.puts "Reading '#{topo.name}'."
	
		# Read clustering log file
#		clustLogFile=topo.cluilogfn
		clustEdgeFile=topo.cluedgefn
		if $opts[:layoutclustering]
#			clustLogFile=topo.layilogfn
			clustEdgeFile=topo.laycluedgefn
		end
#		checkfile(clustLogFile,"clustLogFile")
#		s = IO.read(clustLogFile)
#		topo.n_edge = s[/^#edge *= *(\d+)/, 1].to_i # Number of node-to-node connections
#		topo.n_link = s[/^#link *= *(\d+)/, 1].to_i # Number of node-to-node connections outside racks
#		topo.membership = s[/^membership *= *(.+)$/, 1] # Membership vector
#		if topo.membership
#			topo.membership = topo.membership.scan(/\d+/).collect{|x| x.to_i }
#		else
#			STDERR.puts "Membership vector must exist in '#{clustLogFile}'."
#			exit(1)
#		end
		# Prepare connection matrix and distance matrix
		checkfile(clustEdgeFile,"clustEdgeFile")
		topo.conn = MyMatrix.new.load_edgelist(clustEdgeFile)
		checkfile(topo.laycoordfn,"topo.laycoordfn")
		layout = RackLayout.new(topo.conn.row_size).load_coordinates(topo.laycoordfn)
		if $opts[:euclidean]
			dist = layout.l2norm_matrix
		else
			dist = layout.l1norm_matrix
		end
		# Prepare traffic matrix
		if $opts[:traffic]
			checkfile(topo.trafficfn,"topo.trafficfn")
			MyMatrix.new.load_edgelist(topo.trafficfn).each do |v1, v2, w|
				if membership[v1] == membership[v2]
					topo.traffic_inner += w
				else
					topo.traf[membership[v1], membership[v2]] += w
				end
			end
			topo.conn = topo.traf
		end

		if !config.cabinetyears[topo.year].dist
			config.cabinetyears[topo.year].dist=dist
		end

		raise "Distance matrix is not square (#{dist.row_size} rows, #{dist.col_size} cols)" unless dist.row_size == dist.col_size
		raise "Flow matrix is not square (#{topo.conn.row_size} rows, #{topo.conn.col_size} cols)" unless topo.conn.row_size == topo.conn.col_size
		raise "Unmatch matrix size (dist: #{dist.row_size}x#{dist.col_size}, flow: #{topo.conn.row_size}x#{topo.conn.col_size})" unless dist.row_size == topo.conn.row_size

	} #End of topologies reading
	
	
	q = QAP.new(config)

	# Prepare solver
	case ARGV[5]
	when "sa"
		STDOUT.puts "Cable coeff. is #{ARGV[1]}, Energy coeff. is #{ARGV[2]}, Redundancy coeff. is #{ARGV[3]}\n" 
		q.solver = SaQapSolver.new(100000000, 10,ARGV[4],ARGV[1], ARGV[2], ARGV[3])
	when "ts"
		q.solver = RoTsQapSolver.new
	when "base"
		q.solver = BaselineQapSolver.new(layout, nil)
	else
		puts "Not a correct solver name#{ARGV[4]}!\n"+$help_message
		exit(1)
	end

	if ! $opts[:costnorm] 
		q.costnorm=1
	else
		q.costnorm=$opts[:costnorm] 
	end
	
	if $opts[:usepatchpanels] 
		q.useppanel=1
	else
		q.useppanel=0
	end
		
	#===========================================================
	#   Solve
	#===========================================================
#	if (!File.size?(config.allilogfn) && !$opts[:reproduce]) || $opts[:overwrite]
	for year in 0..$opts[:lastyear].to_i 
          q.currentyear=year
          if  !File.size?("alloc_#{q.currentyear+1}.in")
            open(config.allilogfn, "w") do |logf|
		   	 logf.sync = true
			 q.solve(logf)
		   end
  	  else
		STDOUT.puts "Skipping allocation of year #{year} because result file (alloc_#{q.currentyear+1}.in) already exists!" 
          end
         end
#	elsif File.size?(config.allilogfn)
# 		STDERR.puts "Reproducing the result from '#{logfn}'" unless $opts[:quiet]
#		open(config.allilogfn, "r") do |logf|
#			q.reproduce(logf)
#		end
#	else
#		STDERR.puts "Log file '#{logfn}' is required to reproduce the result."
#		exit(1)
#	end

	#===========================================================
	#   Output
	#===========================================================
	# Display summary
	v1 = q.objective
	v2 = q.cablecost
	v3 = q.power
    v4 = q.length 
	#config.topologies.each{ |topo|	
	#	v4 += topo.traffic_inner *1 # total energy consumption
	#	topo.conn.each do |i, j, w|
	#	 	next if i >= j # unidirectional inter-rack cables
	#		len = q.dist[q.location[i], q.location[j]]
	#		v2 += len * w
	#		v3 += len * w
	#		v4 += len * topo.traf[i, j]
	#	end
	#}
	
	sol = q.solution.join(",")
	timestamp = Time.now.strftime("%Y-%m-%d %H:%M:%S")
	#STDOUT.puts ["Synthesis", q.size, v1, v2, v3, v4].join("\t")
 	STDOUT.puts ["Value","NNod", "Synth$", "Cbl$" ,"Pow", "Aggregated length","Make", "Now"].join("\t ")
	STDOUT.puts ["Synthesis",q.size, v1, v2, v3, v4, q.elapsed, timestamp].join("\t ")
	
	# Write nodewise edgelist and coordinates
	dist=config.cabinetyears[-1].dist
	config.topologies.each{ |topo|
		open(topo.alledgefn, "w") do |f|
			topo.conn.each_link do |v1, v2, w|
				#len = dist[topo.location[topo.membership[v1]], topo.location[topo.membership[v2]]]
				f.printf("%d %d\n", v1, v2)
			end
		end
#		open(topo.allcoordfn, "w") do |f|
#			topo.membership.each_with_index do |rack, node|
#				x, y = 1,1 # layout.coord(topo.location[rack])
#				f.printf("%d %d %d %d\n", node, rack, x, y)
#			end
#		end
	}

	return 
	
	if $opts[:output_histogram]
		tile = layout.tile
		slots = layout.max_length / tile
		hist = Array.new(slots, 0)
		conn.each do |i, j, w|
			len = dist[q.location[i], q.location[j]]
			slot = len / tile
			hist[slot] += w
		end
		legend = Array.new
		slots.times do |slot|
			legend[slot] = slot * tile
		end
		puts ["", legend].join("\t")
		puts [parameter, hist].join("\t")
	end
end


# ラック配置
class RackLayout
	def initialize(n)
		@nrack = n.to_i # ラック数
		@width = RACK_WIDTH # ラック幅 (cm)
		@depth = RACK_DEPTH # ラック奥行＋通路幅 (cm)
		@tile  = @width.gcd(@depth) # 最大公約数
		if @nrack > 0
			@ny = 1 # 奥行方向の設置数
			@nx = @nrack # 幅方向の設置数
		else
			@ny = 0
			@nx = 0
		end
	end
	attr_reader :nrack, :nx, :ny, :width, :depth, :aisle, :tile
	# 部屋の幅
	def room_width
		@nx * @width
	end
	# 部屋の奥行
	def room_depth
		@ny * @depth
	end
	# 部屋の面積
	def room_area
		room_width * room_depth
	end
	# 最長距離
	def max_length
		room_width + room_depth
	end
	# 列ごとにイテレート
	def each_row
		(0...@nrack).each_slice(@nx) do |row|
			yield(row)
		end
	end
	def each_row_with_index
		i = 0
		(0...@nrack).each_slice(@nx) do |row|
			yield(row, i)
			i += 1
		end
	end
	# ラックの座標 [x, y]
	def coord(i)
		[(i % @nx) * @width, (i / @nx) * @depth]
	end
	# マンハッタン距離
	def l1norm(i, j)
		x = (i.to_i % @nx - j.to_i % @nx) * @width
		y = (i.to_i / @nx - j.to_i / @nx) * @depth
		x.abs + y.abs
	end
	# ユークリッド距離
	def l2norm(i, j)
		x = (i.to_i % @nx - j.to_i % @nx) * @width
		y = (i.to_i / @nx - j.to_i / @nx) * @depth
		Math.sqrt(x * x + y * y).round
	end
	# マンハッタン距離行列
	def l1norm_matrix
		a = MyMatrix.new
		@nrack.times do |i|
			@nrack.times do |j|
				a[i, j] = a[j, i] = l1norm(i, j)
			end
		end
		a
	end
	# ユークリッド距離行列
	def l2norm_matrix
		a = MyMatrix.new
		@nrack.times do |i|
			@nrack.times do |j|
				a[i, j] = a[j, i] = l2norm(i, j)
			end
		end
		a
	end
	# 座標ファイルを読み込む
	def load_coordinates(filename)
		rmax = 0
		xmax = 0
		ymax = 0
		STDOUT.puts "Loading coordinates from #{filename}\n"
		IO.foreach(filename) do |line|
			n, r, x, y = line.split # split by whitespaces
			next unless n && r && x && y
			n = n.to_i
			r = r.to_i
			x = x.to_i
			y = y.to_i
			raise "Rack width unmatch with X coordinates" unless x % @width == 0
			raise "Rack depth unmatch with Y coordinates" unless y % @depth == 0
			rmax = r if rmax < r
			xmax = x if xmax < x
			ymax = y if ymax < y
		end
		raise "Number of racks unmatch in edgelist #{@nrack} and in coordinates #{rmax + 1}" unless rmax + 1 == @nrack
		@nx = xmax / @width + 1
		@ny = ymax / @depth + 1
		self
	end
end

class SquareRoomLayout < RackLayout
	def initialize(n)
		super(n)
		# なるべく正方形に近い部屋とする
		if @nrack > 0
			ra = Math.sqrt(@nrack * @width * @depth)
			@ny = (ra / @depth).ceil # 奥行方向の設置数
			@nx = (@nrack.to_f / @ny).ceil      # 幅方向の設置数
			if @ny > 1
				ny2 = @ny - 1
				nx2 = (@nrack.to_f / ny2).ceil
				if nx2 * ny2 < @nx * @ny
					@nx = nx2
					@ny = ny2
				end
			end
		end
	end
end

class SquareMeshLayout < RackLayout
	def initialize(n)
		super(n)
		# ラックを縦横同数ずつ並べる
		if @nrack > 0
			@ny = Math.sqrt(@nrack).ceil   # 奥行方向の設置数
			@nx = (@nrack.to_f / @ny).ceil # 幅方向の設置数
		end
	end
end

# 二次割当問題
class QAP
	def initialize(config)
		@config = config
		@size = 0; config.cabinetyears.each{ |yea| 
			raise "Distance of cabinet year #{yea.inspect} should be non-nil" unless yea.dist!=nil; 
			if @size<yea.dist.row_size
				@size=yea.dist.row_size
			end
			} # 問題サイズ
			
			@dist=MyMatrix.new();
			config.topologies.each{ |topo| 
				@dist.squareconcat( config.cabinetyears[topo.year].dist )			
			}
			@flow=MyMatrix.new();
			config.topologies.each{ |topo| 
				@flow.squareconcat( topo.conn )			
			}
			@location=Array.new
		@solver = nil # ソルバー
		@solution = Array.new# 解 @solution[位置] = 施設
		@location = Array.new(@dist.row_size) # 解 @location[施設] = 位置
		@objective = nil # 目的関数値
		@iterations = nil # 反復回数
		@inverse = false # 距離行列と重み行列を入れ替えて評価するフラグ
		@elapsed = 0 # 計算時間
	end
	attr_reader :size, :config, :dist, :flow, :location
	attr_accessor :solver, :solution, :location, :objective, :iterations, :inverse, :elapsed, :cablecost, :power, :length, :costnorm, :useppanel, :currentyear
	# 求解
	def solve(logf)
		raise "No solver set" unless @solver.is_a?(QapSolver)
		File.open("allocinput.dat", 'w') { |file| file.write(self.to_s) }
		@solver.run(self, logf)
	end
	# ログファイルから再現
	def reproduce(logf)
		raise "No solver set" unless @solver.is_a?(QapSolver)
		File.open("allocinput.dat", 'w') { |file| file.write(self.to_s) }
		@solver.reproduce(self, logf)
	end
	# QAPLIB形式の文字列
	def to_s
	#Stream: <number of years(i.e. distance matrices)    currentYear  normdegree patch>  <distance matrices> <year of topology 1> <probability of topology 1> <communication of matrix 1> ....  
        inyear=@currentyear
		result="#{@config.cabinetyears.length} #{@config.topologies.length}  #{inyear}  #{@costnorm} #{@useppanel} \n"
		@config.cabinetyears.each{ |yea| result+="#{yea.rows} #{yea.columns}\n#{yea.dist}\n"} 
		@config.topologies.each{ |topo| result+="#{topo.year} #{topo.probability} #{topo.name}\n#{topo.conn}\n"} 
		result
		#{@config.dist}\n#{@flow}\n"
	end
	# 評価関数値を計算
	def evaluate(sol)
		sum = 0
		unless @inverse
			@dist.each do |i, j, w|
				sum += w * @flow[sol[i], sol[j]]
			end
		else
			@flow.each do |i, j, w|
				sum += w * @dist[sol[i], sol[j]]
			end
		end
		sum / 2
	end
	# 評価関数値を重みなしで計算
	def evaluate_unweighted(sol)
		sum = 0
		unless @inverse
			@dist.each do |i, j, w|
				sum += w if @flow[sol[i], sol[j]] > 0
			end
		else
			@flow.each do |i, j, w|
 				sum += w if @dist[sol[i], sol[j]] > 0
			end
		end
		sum / 2
	end
end

# 二次割当ソルバー
class QapSolver
	def run(q, logf)
		start_time = Time.now
		logf.puts start_time.strftime("===== %Y-%m-%d %H:%M:%S =====")
		if q.size > 0
		   thiscmd = "#{@cmd} alloc_#{q.currentyear}.in"
		   STDOUT.puts "Running SA command <#{thiscmd}>" 
           logf.puts(thiscmd)
		   #Only need to prepare data for the first year, further allocations will reuse output from previous
           if q.currentyear==0
              inFile=File.open("alloc_0.in",'w');
			  write_input(q,inFile);
			  inFile.close
            end
			IO.popen(thiscmd, "r+") do |pipe|
				pipe.close_write
				read_output(q, pipe, logf)
			end
			q.solution.each_with_index do |x, i|
				q.location[x] = i
			end
		else
			q.iterations = 0
			q.objective = 0
			q.solution = []
			q.location = []
		end
		finish_time = Time.now
		logf.puts finish_time.strftime("===== %Y-%m-%d %H:%M:%S =====")
		q.elapsed = finish_time - start_time
	end
	def reproduce(q, logf)
		read_output(q, logf, nil)
		q.solution.each_with_index do |x, i|
			q.location[x] = i
		end
		logf.rewind
		ts = logf.read.scan(/^===== (\d+-\d+-\d+ \d+:\d+:\d+) =====$/)
		start_time = Time.parse(ts[0][0])
		finish_time = Time.parse(ts[-1][0])
		q.elapsed = finish_time - start_time
	end
end

# GRASP
class GraspQapSolver < QapSolver
	def initialize(mode)
		here = File.dirname(File.expand_path(__FILE__))
		case mode
		when "dense"
			@cmd = "#{here}/gqapd"
		when "sparse"
			@cmd = "#{here}/gqaps"
		else
			raise "Option must be 'dense' or 'sparse'"
		end
	end
	def write_input(q, f)
		f.puts(q)
	end
	def read_output(q, f, logf)
		output = f.tee(logf)
		q.iterations = output[/^ *grasp iterations *: *(\d+)/, 1].to_i
		q.objective = output[/^ *cost of best permutation found *: *(\d+)/, 1].to_i / 2
		q.solution = output[/^ *best permutation found((\s*:( *\d+)+)+)/, 1].scan(/\d+/).collect{|n| n.to_i - 1 }
	end
end

class TaillardQapSolver < QapSolver
	def write_input(q, f)
		f.puts(q)
	end
	def read_output(q, f, logf)
		f.tee(logf).scan(/^@(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+\[([\d,\s]+)\]/).each do |m|
			if !q.iterations || q.iterations < m[0].to_i
				# 反復回数の最大値
				q.iterations = m[0].to_i
			end

			# 目的関数の最小値と、そのときの解
			q.objective = m[1].to_i
			q.cablecost = m[2].to_i
			q.power = m[3].to_i
			q.length = m[4].to_i
			q.config.topologies[m[5].to_i].solution = m[6].scan(/\d+/).collect{|n| n.to_i }
			STDOUT.puts "Solution for topology #{m[5].to_i} is #{q.config.topologies[m[5].to_i].solution}" 
			
		end

		#Aggregate all topology solutions
		q.config.topologies.each{ |topo| 
			q.solution.concat(topo.solution)
			topo.solution.each_with_index do |x, i|
				topo.location[x] = i
			end	
		} 			
		
		STDOUT.puts "Finished reading solution" 
	end
end

# Fast Ant Colony
class FantQapSolver < TaillardQapSolver
	def initialize(n_iteration, n_trial)
		here = File.dirname(File.expand_path(__FILE__))
		@cmd = "#{here}/fant_qap #{n_iteration} #{n_trial}"
	end
end

# Simulated Annealing
class SaQapSolver < TaillardQapSolver
	def initialize(n_iteration, n_trial, limit, cableCoeff, energyCoeff, redundancycoeff)
		here = File.dirname(File.expand_path(__FILE__))
		@cmd = "#{here}/sa_qxap #{n_iteration} #{n_trial} #{limit} #{cableCoeff} #{energyCoeff} #{redundancycoeff}"
	end
end

# Robust Taboo Search
class RoTsQapSolver < TaillardQapSolver
	def initialize
		here = File.dirname(File.expand_path(__FILE__))
		@cmd = "#{here}/tabou_qap"
	end
end

# SCOP
class ScopQapSolver < QapSolver
	def initialize(iteration = 1000000, seed = 1)
		here = File.dirname(File.expand_path(__FILE__))
		@cmd = "#{here}/wcsp -iteration #{iteration} -seed #{seed}"
	end
	def write_input(q, f)
		vars = (0...q.size).collect{|i| "x[#{i}]" }
		vals = (0...q.size).collect{|i| "#{i}" }
		vars.each do |var|
			f.print("variable #{var} in {#{vals.join(',')}}\n")
		end
		f.print("constraint: weight=inf type=alldiff #{vars.join(' ')} ;\n")
		f.print("objective: weight=1 type=quadratic\n")
		(0...(q.size - 1)).each do |i|
			((i + 1)...q.size).each do |j|
				(0...q.size).each do |k|
					(0...q.size).each do |l|
						if k != l
							f.print("#{q.flow[i, j] * q.dist[k, l]}(#{vars[i]},#{vals[k]})(#{vars[j]},#{vals[l]})\n")
						end
					end
				end
			end
		end
		f.print("<= 0\n")
	end
	def read_output(q, f, logf)
		f.tee(logf).each_line("") do |s| # paragraph mode
			case s
			when /^# iteration =/
				s.scan(/^#\s*(\w+)\s*=\s*(\d+)\/(\d+)/).each do |m|
					q.iterations = m[1].to_i if m[0] == "iteration"
				end
			when /^\[best solution\]/
				s.scan(/^x\[(\d+)\]:\s*(\d+)$/).each do |m|
					q.solution[m[0].to_i] = m[1].to_i
				end
			when /^\[Violated constraints\]/
				s.scan(/^(\w+):\s*(\d+)$/).each do |m|
					q.objective = m[1].to_i if m[0] == "objective"
				end
			end
		end
		q.inverse = true
	end
end

# Baseline
class BaselineQapSolver < QapSolver
	def initialize(layout, method = nil)
		raise "No layout given" unless layout.is_a?(RackLayout)
		@lay = layout
		@method = method
	end
	def run(q, logf)
		if q.size > 0
			case @method
			when "oneway"
				q.solution = sol_oneway
			when "zigzag"
				q.solution = sol_zigzag
			else
				sols = [sol_oneway, sol_zigzag]
				q.solution = sols.min_by{|sol| q.evaluate(sol) }
			end
			q.iterations = 0
			q.objective = q.evaluate(q.solution)
			q.solution.each_with_index do |x, i|
				q.location[x] = i
			end
		else
			q.iterations = 0
			q.objective = 0
			q.solution = []
			q.location = []
		end
	end
	def sol_oneway
		sol = Array.new(@lay.nrack)
		@lay.each_row_with_index do |row, i|
			sol[i * @lay.nx, row.length] = row
		end
		sol
	end
	def sol_zigzag
		sol = Array.new(@lay.nrack)
		@lay.each_row_with_index do |row, i|
			row = row.reverse if i.odd?
			sol[i * @lay.nx, row.length] = row
		end
		sol
	end
end

class IO
	# ログファイルに書き出しつつ文字列として返す
	def tee(f)
		str = ""
		self.each_line do |line|
			line = line.chomp
			f.puts(line) if f
			str << line << "\n"
		end
		str
	end
end

# 本体
main if $0 == __FILE__
