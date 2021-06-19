#!/usr/bin/ruby -Ku
# -*- coding: utf-8 -*-
#
# 	Control the complete flow for allocating extensible multiple topologies to yearly sets of cabinets
#  config file contains the configuration for requested topologies and cabinets sizing 
#  Syntax details are given in the sample config file 

# 行列
# 行番号・列番号は0オリジン
class MyMatrix
	include Enumerable
	def initialize
		@data = Hash.new{|hash, key|
			hash[key] = Hash.new(0) # 行データのデフォルト値
		}
		@row_size = 0
		@col_size = 0
	end
	attr_reader :row_size, :col_size
	def initialize_copy(org)
		@data = Hash.new{|hash, key|
			hash[key] = Hash.new(0) # デフォルト値
		}
		org.each do |i, j, val|
			@data[i][j] = val
		end
	end
	def [](i, j)
		@data[i][j]
	end
	def []=(i, j, val)
		raise TypeError unless i.is_a?(Integer) && j.is_a?(Integer) && (val.is_a?(Numeric) || val==nil)
		@data[i][j] = val
		@row_size = i + 1 if row_size < i + 1
		@col_size = j + 1 if col_size < j + 1
		self
	end
	# スカラー和
	def +(other)
		raise TypeError unless other.is_a?(Numeric)
		a = self.class.new
		@row_size.times do |i|
			@col_size.times do |j|
				a[i, j] = @data[i][j] + other
			end
		end
		a
	end
	# スカラー積
	def *(other)
		raise TypeError unless other.is_a?(Numeric)
		a = self.class.new
		@data.each do |i, r|
			r.each do |j, val|
				a[i, j] = val * other
			end
		end
		a
	end
	# 各要素にブロックを適用した結果を新しい行列で返す
	def map
		a = self.class.new
		@data.each do |i, r|
			r.each do |j, val|
				a[i, j] = yield(val)
			end
		end
		a
	end
	def squareconcat(b)
		b.each{ |i,j,val|
				self[@row_size+i,@col_size+j] = val
		}
		@row_size= @row_size + b.row_size
		@col_size= @col_size + b.col_size
		self
	end
	def empty?
		@data.empty?
	end
	def each
		@data.each do |i, r|
			r.each do |j, val|
				yield(i, j, val)
			end
		end
	end
	def each_link
		@data.each do |i, r|		
			r.each do |j, val|
				if val!=0 && (i<j ||  self[j,i]==0 )
					yield(i, j, val)
				end
			end
		end
	end
	def each_index
		@row_size.times do |i|
			@col_size.times do |j|
				yield(i, j)
			end
		end
	end
	def each_index_upper(strict = false)
		raise "Square matrix expected" unless @row_size == @col_size
		d = (strict) ? 1 : 0
		(0...(@row_size - d)).each do |i|
			((i + d)...@col_size).each do |j|
				yield(i, j)
			end
		end
	end
	def along_row(i)
		@data[i].each do |j, val|
			yield(j, val)
		end
	end
	def along_col(j)
		@data.each do |i, r|
			yield(i, r[j])
		end
	end
	def to_s
		a = Array.new
		@row_size.times do |i|
			b = Array.new
			@col_size.times do |j|
				b.push(@data[i][j].to_s)
			end
			a.push(b.join(" "))
		end
		a.join("\n")
	end
	# スペース区切り文字列を読む
	def scan(str)
		str.each_with_index do |line, i|
			line.split.each_with_index do |val, j|
				self[i.to_i, j.to_i] = val.to_i
			end
		end
		self
	end
	# エッジリストを隣接行列として読み込む
	def load_edgelist(filename, undirected = true, preamble=0)
		STDOUT.puts "Loading edge list from #{filename}\n"
          
		File.open(filename){ |file| file.each do |line|
                                if preamble>0
                                  preamble=preamble-1
                                  next
                                end
				i, j, val = line.split # split by whitespaces
				next unless i && j
				i = i.to_i
				j = j.to_i
				val = (val) ? val.to_i : 1
				self[i, j] = val
				self[j, i] = val if undirected
			end
		}
		self
	end
end

class CabinetYear
	attr_accessor :rows, :columns
	attr_accessor :dist
	
	def initialize(line)
		@rows=line.split()[0].to_i()
		@columns=line.split()[1].to_i()
	end
	
	def cabinetcount
		@rows*@columns
	end
	
end

class Topology

	attr_accessor :name, :year, :probability
	attr_accessor :n_edge, :n_tube,  :n_link,  :membership
	attr_accessor :conn, :solution, :location
	attr_accessor :laycoordfn, :layedgefn, :laycluedgefn, :cluedgefn, :alledgefn, :allcoordfn, :layilogfn, :cluilogfn, :allilogfn, :layglogfn, :cluglogfn, :allglogfn #, :edgfn,  :mapfn, :outfn,  :clufn
	attr_accessor :traffic_inner, :traf
	
	def initialize(line)
		@name , @year, @probability = line.split()
		@year=@year.to_i
		@probability=@probability.to_f
		@traffic_inner = 0
		@traf = MyMatrix.new
		@location=Array.new		
		@solution=Array.new
	end
	

	def setfilenames(layprefix,laysuffix,clustersuffix)

		@laycoordfn="#{layprefix}.coord"		
		@allcoordfn="#{layprefix}+#{name}#{laysuffix}#{clustersuffix}.coord"
		
		@layedgefn="#{layprefix}+#{name}#{laysuffix}.edges"		
		@laycluedgefn="#{layprefix}+#{name}#{laysuffix}.clu.edges"		
	  	@cluedgefn="#{layprefix}+#{name}#{laysuffix}#{clustersuffix}.edges"		
	  	@alledgefn="#{layprefix}+#{name}#{laysuffix}#{clustersuffix}.all.edges"		

		@layilogfn ="#{layprefix}+#{name}#{laysuffix}.clu.log"
	  	@cluilogfn="#{layprefix}+#{name}#{laysuffix}#{clustersuffix}.log"		
	  	@allilogfn="#{layprefix}+#{name}#{laysuffix}#{clustersuffix}.all.log"		

		@layglogfn ="#{layprefix}+#{name}#{laysuffix}_layout.log"
	  	@cluglogfn="#{layprefix}+#{name}#{laysuffix}#{clustersuffix}_cluster.log"		
	  	@allglogfn="#{layprefix}+#{name}#{laysuffix}#{clustersuffix}.all.log"		
	  		
		#@edgfn = "#{name}.edges"
		#@mapfn = "#{dimension}.coord"
		#@outfn = "#{parameter}.edges"
		#@clufn = "#{parameter}.clu.edges"
	end
end


class MyConfig

	attr_accessor :seed, :cabinetyears,	:nodepercabinet,	:topologies, :layoutoptions, 	:clusteringoptions, 	:allocationoptions, :allilogfn, :postoptins, :postilogfn;
		 
		 	
	
	
	def initialize(configname, seed )
		if !File.exist?(configname)
			STDERR.puts "Configuration file '#{configname}' does not exist. "
			exit(0)
		end
	
		#Read the configuration file
		@seed=seed
		@cabinetyears=Array.new
		@nodepercabinet=0
		@topologies=Array.new
		@layoutoptions=Array.new
		@clusteringoptions=Array.new
		@allocationoptions=Array.new
		@allilogfn="alloc.log"
		@postoptions=Array.new
		@postilogfn="post.log"
		configpart=""
		STDOUT.puts "Reading (#{configname}) configuration file.."
		File.foreach(configname){ |line|
			line=line.chomp()
			if line.length	!= 0 && line[0] !='#'
				STDOUT.puts "Reading #{line}.."
				case  line.split()[0]
				when "CABINETS"
					configpart="CABINETS"
					@nodepercabinet=line.split()[1]
				when "TOPOLOGIES"
					configpart="TOPOLOGIES"
				when "LAYOUT"
					configpart="LAYOUT"
					@layoutoptions=line.split()[1..-1]
				when "CLUSTERING"
					configpart="CLUSTERING"
					@clusteringoptions=line.split()[1..-1]
				when "ALLOCATION"
					configpart="ALLOCATION"
					@allocationoptions=line.split()[1..-1]
				when "POST"
					configpart="POST"
					@postoptions=line.split()[1..-1]
				else
					#Read the line according to the current topology part
					case configpart
					when "CABINETS"
						@cabinetyears.push(CabinetYear.new(line))
					when "TOPOLOGIES"
						@topologies.push(Topology.new(line))
					else
						STDERR.puts "Configuration line during configuration part #{configpart}  is ignored (#{line}).."  unless line.split(" ").length==0
					end
				end
			end
		} # end of configuration file reading	
		
		#Configure filenames
		@topologies.each{ |topo|
  			cabinetcount=cabinetyears[topo.year].cabinetcount
  			nnodes=cabinetcount.to_i*nodepercabinet.to_i;
			layprefix="#{nnodes}r#{nodepercabinet}"
			laysuffix="[#{seed}]"
			clustersuffix=".#{clusteringoptions[0]}-#{nodepercabinet}"
			topo.setfilenames(layprefix,laysuffix,clustersuffix)
		}
		
	end
	
end
